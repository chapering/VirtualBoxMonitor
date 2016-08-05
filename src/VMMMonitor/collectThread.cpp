/****************************************************************************
 * @file collectThread.cpp
 *
 * collectThread class implementation 
 *
 * @creation	Nov. 24 2012
 * 
 * Copyright (C) 2012-2013 
 *
 * This file is part of the final project for gradOS Fall 2012
 *
****************************************************************************/
/* Vbox includes */
# include <VBox/com/com.h>
# include <VBox/com/string.h>
# include <VBox/com/Guid.h>
# include <VBox/com/array.h>
# include <VBox/com/ErrorInfo.h>
# include <VBox/com/errorprint.h>
# include <VBox/com/EventQueue.h>

# include <VBox/com/VirtualBox.h>

#include <VBox/err.h>
#include <VBox/version.h>

#include <iprt/asm.h>
#include <iprt/stream.h>
#include <iprt/string.h>
#include <iprt/time.h>
#include <iprt/thread.h>
#include <iprt/buildconfig.h>
#include <iprt/ctype.h>
#include <iprt/initterm.h>
#include <iprt/path.h>

#include <set>
#include <utility>

/* local includes */
#include "collectThread.h"

#ifdef Q_WS_MAC
# include "VBoxUtils.h"
# if MAC_LEOPARD_STYLE
#  define VBOX_GUI_WITH_TOOLBAR_SETTINGS
# endif /* MAC_LEOPARD_STYLE */
#endif /* Q_WS_MAC */

using namespace com;

// funcs
///////////////////////////////////////////////////////////////////////////////
static Bstr toBaseName(Utf8Str& aFullName)
{
    char *pszRaw = aFullName.mutableRaw();
    char *pszSlash = strrchr(pszRaw, '/');
    if (pszSlash)
    {
        *pszSlash = 0;
        aFullName.jolt();
    }
    return Bstr(aFullName);
}

static int parseFilterParameters(const char* metriclist,
                                 ComPtr<IVirtualBox> aVirtualBox,
                                 ComSafeArrayOut(BSTR, outMetrics),
                                 ComSafeArrayOut(IUnknown *, outObjects))
{
    HRESULT rc = S_OK;
    com::SafeArray<BSTR> retMetrics(1);
    com::SafeIfaceArray <IUnknown> retObjects;

    Bstr metricNames, baseNames;

    /* Metric list */
    if (strlen(metriclist) >= 1)
        metricNames = metriclist;
    else
    {
        metricNames = L"*";
        baseNames = L"*";
    }
    metricNames.cloneTo(&retMetrics[0]);

    /* Object name */
	ComPtr<IHost> host;
	CHECK_ERROR(aVirtualBox, COMGETTER(Host)(host.asOutParam()));
	retObjects.reset(1);
	host.queryInterfaceTo(&retObjects[0]);

    retMetrics.detachTo(ComSafeArrayOutArg(outMetrics));
    retObjects.detachTo(ComSafeArrayOutArg(outObjects));

    return rc;
}

// collectThread class
///////////////////////////////////////////////////////////////////////////////
ComPtr<IVirtualBox> collectThread::m_virtualBox = NULL;
ComPtr<IPerformanceCollector> collectThread::m_performanceCollector = NULL;
ComPtr<ISession> collectThread::m_session = NULL;

//! [0]
collectThread::collectThread(QObject *parent)
    : QThread(parent)
    /* Protected variables: */
    /* priviate variables: */
	, m_pplot(NULL)
	, m_strMachine("host")
	, m_strMetriclist("")
	, m_nPeriod(1)
	, m_nSamples(1)
	, m_bDetached(false)
	, m_bListMatches(false)
	, m_bPause(false)
	, m_bAbort(false)
{
}
//! [0]

//! [1]
collectThread::~collectThread()
{
    m_mutex.lock();
    m_bAbort = true;
    m_condition.wakeOne();
    m_mutex.unlock();
}
//! [1]

int collectThread::enable()
{
    com::SafeArray<BSTR>          metrics(1);
    com::SafeIfaceArray<IUnknown> objects;

    Bstr metricNames, baseNames;

    /* Metric list */
    if (m_strMetriclist.length() >= 1)
        metricNames = m_strMetriclist.toStdString().c_str();
    else
    {
        metricNames = L"*";
        baseNames = L"*";
    }
    metricNames.cloneTo(&metrics[0]);

	ComPtr<IHost> host;
    HRESULT rc = S_OK;
	CHECK_ERROR(m_virtualBox, COMGETTER(Host)(host.asOutParam()));
	objects.reset(1);
	host.queryInterfaceTo(&objects[0]);

    com::SafeIfaceArray<IPerformanceMetric> affectedMetrics;
    CHECK_ERROR(m_performanceCollector,
        EnableMetrics(ComSafeArrayAsInParam(metrics),
                      ComSafeArrayAsInParam(objects),
                      ComSafeArrayAsOutParam(affectedMetrics)));
    if (FAILED(rc))
        return -1;

    return 0;
}

int collectThread::disable()
{
    com::SafeArray<BSTR>          metrics(1);
    com::SafeIfaceArray<IUnknown> objects;

    Bstr metricNames, baseNames;

    /* Metric list */
    if (m_strMetriclist.length() >= 1)
        metricNames = m_strMetriclist.toStdString().c_str();
    else
    {
        metricNames = L"*";
        baseNames = L"*";
    }
    metricNames.cloneTo(&metrics[0]);

	ComPtr<IHost> host;
    HRESULT rc = S_OK;
	CHECK_ERROR(m_virtualBox, COMGETTER(Host)(host.asOutParam()));
	objects.reset(1);
	host.queryInterfaceTo(&objects[0]);

    com::SafeIfaceArray<IPerformanceMetric> affectedMetrics;
    CHECK_ERROR(m_performanceCollector,
        DisableMetrics(ComSafeArrayAsInParam(metrics),
                      ComSafeArrayAsInParam(objects),
                      ComSafeArrayAsOutParam(affectedMetrics)));
    if (FAILED(rc))
        return -1;

    return 0;
}

void collectThread::halt()
{
    m_mutex.lock();
    m_bAbort = true;
    m_condition.wakeOne();
    m_mutex.unlock();
}

void collectThread::pause()
{
    m_mutex.lock();
    m_bPause = true;
    m_mutex.unlock();
}

void collectThread::restart()
{
    m_mutex.lock();
    m_bPause = false;
	m_condition.wakeOne();
    m_mutex.unlock();
}

void collectThread::setParas(uint32_t samples, uint32_t period, bool detached, bool listmatches)
{
	m_mutex.lock();
	this->m_nSamples = samples;
	this->m_nPeriod = period;
	this->m_bDetached = detached;
	this->m_bListMatches = listmatches;
	m_mutex.unlock();
}

//! [2]
void collectThread::collect(const char* machine, const char* metriclist, QCustomPlot* pplot)
{
    //QMutexLocker locker(&m_mutex);

	this->m_strMachine = machine;
	this->m_strMetriclist = metriclist;
	this->m_pplot = pplot;

	//printf("In the thread for %s\n", metriclist);

    if (!isRunning()) {
		if (0 != setup()) {
			RTPrintf("setting up failed...\n");
			return;
		}
        start(LowPriority);
    } else {
        m_bPause = false;
        m_condition.wakeOne();
		// for monitoring real-time change of the VMM metrics, we may need to setup each time before querying
		if ( m_strMetriclist.contains("VMM") ) setup();
    }
}

int collectThread::setup()
{
	m_mutex.lock();
	/* when read these parameters, they should not be written anywhere else */
	const char* metriclist = this->m_strMetriclist.toStdString().c_str();
	uint32_t samples = this->m_nSamples;
	uint32_t period = this->m_nPeriod;
	bool isDetached = this->m_bDetached;
	ComPtr<IVirtualBox> virtualBox = this->m_virtualBox;
    ComPtr<IPerformanceCollector> performanceCollector = this->m_performanceCollector;
	m_mutex.unlock();

	HRESULT rc;
	com::SafeArray<BSTR>          metrics;
	com::SafeIfaceArray<IUnknown> objects;

	int i = 0;

	/*
	const char* metriclist = "";
	Bstr metricNames, baseNames;

	if (strlen(metriclist) >= 1)
		metricNames = metriclist;
	else
	{
		metricNames = L"*";
		baseNames = L"*";
	}
	metricNames.cloneTo(&metrics[0]);

	ComPtr<IHost> host;
	CHECK_ERROR(pDlg->m_virtualBox, COMGETTER(Host)(host.asOutParam()));
	objects.reset(1);
	host.queryInterfaceTo(&objects[0]);
	*/
	rc = parseFilterParameters(metriclist, virtualBox,
							   ComSafeArrayAsOutParam(metrics),
							   ComSafeArrayAsOutParam(objects));
	if (FAILED(rc))
		return -1;

	com::SafeIfaceArray<IPerformanceMetric> metricInfo;

	CHECK_ERROR(performanceCollector,
		GetMetrics(ComSafeArrayAsInParam(metrics),
				   ComSafeArrayAsInParam(objects),
				   ComSafeArrayAsOutParam(metricInfo)));

	std::set<std::pair<ComPtr<IUnknown>,Bstr> > baseMetrics;
	ComPtr<IUnknown> objectFiltered;
	Bstr metricNameFiltered;
	for (i = 0; i < (int)metricInfo.size(); i++)
	{
		CHECK_ERROR(metricInfo[i], COMGETTER(Object)(objectFiltered.asOutParam()));
		CHECK_ERROR(metricInfo[i], COMGETTER(MetricName)(metricNameFiltered.asOutParam()));
		Utf8Str baseMetricName(metricNameFiltered);
		baseMetrics.insert(std::make_pair(objectFiltered, toBaseName(baseMetricName)));
	}
	com::SafeArray<BSTR>          baseMetricsFiltered(baseMetrics.size());
	com::SafeIfaceArray<IUnknown> objectsFiltered(baseMetrics.size());
	std::set<std::pair<ComPtr<IUnknown>,Bstr> >::iterator it;
	i = 0;
	for (it = baseMetrics.begin(); it != baseMetrics.end(); ++it)
	{
		it->first.queryInterfaceTo(&objectsFiltered[i]);
		Bstr(it->second).detachTo(&baseMetricsFiltered[i++]);

		//RTPrintf("BaseMetrics: %ls\n", Bstr(it->second).raw());
	}
	com::SafeIfaceArray<IPerformanceMetric> affectedMetrics;
	CHECK_ERROR(performanceCollector,
		SetupMetrics(ComSafeArrayAsInParam(baseMetricsFiltered),
					 ComSafeArrayAsInParam(objectsFiltered), period, samples,
					 ComSafeArrayAsOutParam(affectedMetrics)));
	if (FAILED(rc))
		return -2;

	if (!affectedMetrics.size())
		return -3;

	m_dataX.resize(MAX_X_POINTS+1);
	for (i=0; i<=MAX_X_POINTS; i+=1)
	{
		m_dataX[i] = i*1.0;
	}

	if (isDetached)
	{
		RTMsgWarning("The background process holding collected metrics will shutdown\n"
					 "in few seconds, discarding all collected data and parameters.");
		return 0;
	}

	// debug
	/*
	RTPrintf("%d Metrics to be queried\n", metrics.size());
	for (i = 0; i< (int)metrics.size();i++)
	{
		RTPrintf("Metrics to be queried %d : %ls\n", i+1, Bstr(metrics[i]).raw());
	}

	RTPrintf("%d objects to be queried\n", objects.size());
	ComPtr<IHost> __host = objects[0];
	if (!__host.isNull())
		RTPrintf("object to be queried : host\n");
	*/
	return 0;
}
//! [2]

//! [3]
void collectThread::run()
{
	HRESULT rc;
	com::SafeArray<BSTR>          metrics;
	com::SafeIfaceArray<IUnknown> objects;

	/* when read these parameters, they should not be written anywhere else */
	m_mutex.lock();
	const char* metriclist = this->m_strMetriclist.toStdString().c_str();
	m_mutex.unlock();

	//const char* metriclist = "";
	rc = parseFilterParameters(metriclist, m_virtualBox,
							   ComSafeArrayAsOutParam(metrics),
							   ComSafeArrayAsOutParam(objects));
	if (FAILED(rc))
		return;

	int pos = 0;
	//bool bFull = false;
	//const char* separator = "";
	double yv;

    forever {
		m_mutex.lock();
		/* when read these parameters, they should not be written anywhere else */
		uint32_t period = this->m_nPeriod;
		ComPtr<IVirtualBox> virtualBox = this->m_virtualBox;
		ComPtr<IPerformanceCollector> performanceCollector = this->m_performanceCollector;
		QCustomPlot* pplot = this->m_pplot;
		if ( m_bAbort ) {
			m_mutex.unlock();
			return;
		}

		m_mutex.unlock();

		RTThreadSleep(period * 1000); // Sleep for 'period' seconds

		com::SafeArray<BSTR>          retNames;
		com::SafeIfaceArray<IUnknown> retObjects;
		com::SafeArray<BSTR>          retUnits;
		com::SafeArray<ULONG>         retScales;
		com::SafeArray<ULONG>         retSequenceNumbers;
		com::SafeArray<ULONG>         retIndices;
		com::SafeArray<ULONG>         retLengths;
		com::SafeArray<LONG>          retData;
		CHECK_ERROR (performanceCollector, QueryMetricsData(ComSafeArrayAsInParam(metrics),
												 ComSafeArrayAsInParam(objects),
												 ComSafeArrayAsOutParam(retNames),
												 ComSafeArrayAsOutParam(retObjects),
												 ComSafeArrayAsOutParam(retUnits),
												 ComSafeArrayAsOutParam(retScales),
												 ComSafeArrayAsOutParam(retSequenceNumbers),
												 ComSafeArrayAsOutParam(retIndices),
												 ComSafeArrayAsOutParam(retLengths),
												 ComSafeArrayAsOutParam(retData)) );

		//RTPrintf("%d metric results returned\n", retNames.size());
		for (unsigned j = 0; j < retNames.size(); j++)
		{
			Bstr metricUnit(retUnits[j]);
			Bstr metricName(retNames[j]);
			/*
			RTPrintf("%-20ls ", metricName.raw());
			for (unsigned k = 0; k < retLengths[j]; k++)
			{
				if (retScales[j] == 1)
					RTPrintf("%s%d %ls", separator, retData[retIndices[j] + k], metricUnit.raw());
				else
					RTPrintf("%s%d.%02d%ls", separator, retData[retIndices[j] + k] / retScales[j],
							 (retData[retIndices[j] + k] * 100 / retScales[j]) % 100, metricUnit.raw());
			}
			RTPrintf("\n");

			printf("in the thread for %s, ", m_strMetriclist.toStdString().c_str());
			RTPrintf("current metric queried: %ls\n", metricName.raw());
			*/
			QCPGraph* pgraph = __locateGraphByMetricName( metricName.raw() );
			if ( NULL == pgraph )
			{
				//RTPrintf("Weird! designated metric not queried?: %ls\n", metricName.raw());
				continue;
			}

			// for these two metrics, retLengths[j] will both be 1
			if (retScales[j] == 1)
				yv = retData[retIndices[j]];
			else
				yv = retData[retIndices[j]] / retScales[j] + (retData[retIndices[j]] * 100 / retScales[j]) % 100 * 0.01;

			pgraph->addData(m_dataX[pos], yv);
			RTPrintf("collected value for %ls = %2d%ls\n", metricName.raw(), (int)yv, metricUnit.raw());

			pos ++;
			if (pos % 3 == 0)
			{
				for(int k=0; k<m_pplot->graphCount();++k) {
					m_pplot->graph(k)->rescaleAxes(true, false);
				}
				/*
				if ( bFull )
				{
					pDlg->qcpMem->graph(0)->removeDataBefore( dataX[5] );
					pDlg->qcpMem->graph(1)->removeDataBefore( dataX[5] );
				}
				*/
				emit sigDataReady(pplot);
			}

			if ( pos >= MAX_X_POINTS )
			{
				RTPrintf("Now Refreshing Metrics being collected...\n");
				for(int k=0; k<m_pplot->graphCount();++k) {
					m_pplot->graph(k)->clearData();
				}
				/*
				pDlg->qcpMem->graph(0)->rescaleAxes(true, false);
				pDlg->qcpMem->graph(1)->rescaleAxes(true, false);
				pDlg->qcpMem->replot();
				*/
				pos = 0;
				//bFull = true;
				emit sigDataReady(pplot);
			}

			m_mutex.lock();
			if ( m_bPause ) {
				m_mutex.unlock();
				break;
			}
			m_mutex.unlock();
		}
//! [3]

//! [4]
        m_mutex.lock();
//! [4] 
//! [5]
        if (m_bPause)
            m_condition.wait(&m_mutex);
        m_bPause = false;
        m_mutex.unlock();
    }
}
//! [5]

//! [6]
int collectThread::__init()
{
    /*
     * Initialize COM.
     */
    using namespace com;
    HRESULT hrc = com::Initialize();
    RTEXITCODE rcExit = RTEXITCODE_FAILURE;
    if (hrc == NS_ERROR_FILE_ACCESS_DENIED)
    {
        char szHome[RTPATH_MAX] = "";
        com::GetVBoxUserHomeDirectory(szHome, sizeof(szHome));
        return RTMsgErrorExit(RTEXITCODE_FAILURE,
               "Failed to initialize COM because the global settings directory '%s' is not accessible!", szHome);
    }
    if (FAILED(hrc))
        return RTMsgErrorExit(RTEXITCODE_FAILURE, "Failed to initialize COM!");

	hrc = m_virtualBox.createLocalObject(CLSID_VirtualBox);
	if (FAILED(hrc))
	{
		RTMsgError("Failed to create the VirtualBox object!");
	}
	else
	{
		hrc = m_session.createInprocObject(CLSID_Session);
		if (FAILED(hrc))
			RTMsgError("Failed to create a session object!");
	}
	if (FAILED(hrc))
	{
		com::ErrorInfo info;
		if (!info.isFullAvailable() && !info.isBasicAvailable())
		{
			com::GluePrintRCMessage(hrc);
			RTMsgError("Most likely, the VirtualBox COM server is not running or failed to start.");
		}
		else
			com::GluePrintErrorInfo(info);
		return rcExit;
	}

    HRESULT rc;
    CHECK_ERROR(m_virtualBox, COMGETTER(PerformanceCollector)(m_performanceCollector.asOutParam()));

	return rcExit = RTEXITCODE_SUCCESS;
}

void collectThread::__exit()
{
	m_session->UnlockMachine();
	EventQueue::getMainEventQueue()->processEventQueue(0);
    com::Shutdown();
}
//! [6]

QCPGraph* collectThread::__locateGraphByMetricName(CBSTR metric)
{
	if ( NULL == m_pplot ) return NULL;
	QStringList allMetrics = m_strMetriclist.split (',');
	int idx = -1;
	for (int j = 0; j < allMetrics.size(); j++) {
		//printf("metric to be compared with: %s\n",allMetrics[j].toStdString().c_str());
		if (Bstr(allMetrics[j].toStdString().c_str()) == metric ) {
			idx = j;
		}
	}
	if ( -1 == idx ) {
		return NULL;
	}
	return m_pplot->graph(idx);
}
//
/* vim set ts=8 sts=8 tw=80 */

