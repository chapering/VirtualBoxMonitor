/****************************************************************************
 * @file collectThread.h
 *
 * collectThread class declaration
 *
 * @purpose collect performance metrics specified by the invoking thread;
 *
 * @note	just collect data, do nothing about rendering, which is not permitted 
 *			outside of the main QtGui thread
 *
 * @creation	Nov. 24 2012
 *
 * Copyright (C) 2012-2013 
 *
 * This file is part of the final project for gradOS Fall 2012
 *
****************************************************************************/

#ifndef collectThread_H
#define collectThread_H

#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include <QString>
#include <qcustomplot.h>

/* VBox includes: */
#include <VBox/com/com.h>
#include <VBox/com/ptr.h>
#include <VBox/com/VirtualBox.h>
#include <VBox/com/array.h>

#include <iprt/types.h>
#include <iprt/message.h>
#include <iprt/stream.h>

QT_BEGIN_NAMESPACE
class QImage;
class QPixmap;
QT_END_NAMESPACE

#define	MAX_X_POINTS 60

//! [0]
class collectThread : public QThread
{
    Q_OBJECT

public:
    collectThread(QObject *parent = 0);
    ~collectThread();

	int enable();
	int disable();
	void halt();
	void pause();
	void restart();

	/* the trigger of this thread used by the main QtGui thread */
	void collect(const char* machine, const char* metriclist, QCustomPlot* pplot);

	/* extra settings */
	void setParas(uint32_t samples=1, uint32_t period=1, bool detached=false, bool listmatches=false);

signals:
	/* will emit this signal when data are ready for updating the graphics */
    void sigDataReady(QCustomPlot* pplot);

protected:
	int setup();
    void run();

protected:
    /* Protected variables: */
	static ComPtr<IVirtualBox> m_virtualBox;
    static ComPtr<IPerformanceCollector> m_performanceCollector;
	static ComPtr<ISession> m_session;

public:
	/* initialize IVirtualBox COM context */
	static int	__init();
	static void __exit();

private:
	/* locate an exact graph by a single metric name */
	QCPGraph* __locateGraphByMetricName(CBSTR metric);

private:
    QMutex m_mutex;
    QWaitCondition m_condition;

	QCustomPlot* m_pplot;

	QString m_strMachine;
	QString m_strMetriclist;
	QVector<double> m_dataX;

    uint32_t m_nPeriod;
	uint32_t m_nSamples;

    bool m_bDetached;
	bool m_bListMatches;

    bool m_bPause;
    bool m_bAbort;
};
//! [0]

#endif
