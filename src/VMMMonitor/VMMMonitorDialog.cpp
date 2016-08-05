/* $Id: VMMMonitorDialog.cpp $ */
/** @file
 *
 * VBox frontends: Qt GUI ("VirtualBox"):
 * VMMMonitorDialog class implementation
 */

/*
 * Copyright (C) 2012-2013 
 *
 * This file is part of the final project for gradOS Fall 2012
 */
#include <set>
#include <utility>
#include <unistd.h>

/* Local includes */
#include "VMMMonitorDialog.h"

/* VMMMonitor Dialog Constructor: */
VMMMonitorDialog::VMMMonitorDialog(QWidget *pParent)
    /* Parent class: */
    //: QMainWindow(pParent)
    : QIWithRetranslateUI<QIMainDialog>(pParent)
    /* Common variables: */
{
    /* Apply UI decorations: */
	Ui::VMMMonitorDialog::setupUi(this);

    /* Page-title font is derived from the system font: */
    QFont pageTitleFont = font();
    pageTitleFont.setBold(true);
    pageTitleFont.setPointSize(pageTitleFont.pointSize() + 2);
	setFont( pageTitleFont );

    /* Setup whatsthis stuff: */
    qApp->installEventFilter(this);

    /* Setup connections: */
	/*
    connect(m_pSelector, SIGNAL(categoryChanged(int)), this, SLOT(sltCategoryChanged(int)));
    connect(m_pButtonBox, SIGNAL(helpRequested()), &msgCenter(), SLOT(sltShowHelpHelpDialog()));
	*/

    /* Translate UI: */
    retranslateUi();

	setWindowTitle("Performance Monitor of VirtualBox VMM");
	tabWidget->setTabText(0, "CPU/Mem Usage");
	tabWidget->setTabText(1, "Disk/Net I/O");
	tabWidget->setTabText(2, "Per-VM CPU/Mem");

	qcpMem->legend->setVisible(true);
	qcpMem->legend->setFont(QFont("Helvetica", 12));
	qcpMem->legend->setPositionStyle(QCPLegend::psTopRight);
	addGraph(qcpMem, "Total", QCPGraph::lsLine, QCP::ssCircle, 10);
	addGraph(qcpMem, "VMM", QCPGraph::lsLine, QCP::ssSquare, 20);

	qcpCPU->legend->setVisible(true);
	qcpCPU->legend->setFont(QFont("Helvetica", 12));
	qcpCPU->legend->setPositionStyle(QCPLegend::psTopRight);
	addGraph(qcpCPU, "Total", QCPGraph::lsLine, QCP::ssCircle, 10);
	addGraph(qcpCPU, "VMM", QCPGraph::lsLine, QCP::ssSquare, 20);

	qcpIO->legend->setVisible(true);
	qcpIO->legend->setFont(QFont("Helvetica", 12));
	qcpIO->legend->setPositionStyle(QCPLegend::psTopRight);
	addGraph(qcpIO, "Disk", QCPGraph::lsLine, QCP::ssCircle, 20);
	addGraph(qcpIO, "NetRecv", QCPGraph::lsLine, QCP::ssSquare, 10);
	addGraph(qcpIO, "NetSend", QCPGraph::lsLine, QCP::ssStar, 30);

	// graph axis : label, unit and range
	qcpMem->xAxis->scaleRange(1.0, 30.0);
	qcpMem->xAxis->setLabel(tr("Time(second)"));
	qcpMem->yAxis->scaleRange(0.5, 1024*1024*1.0);
	qcpMem->yAxis->setLabel(tr("Memory(KB)"));

	qcpCPU->xAxis->scaleRange(1.0, 30.0);
	qcpCPU->xAxis->setLabel(tr("Time(second)"));
	qcpCPU->yAxis->scaleRange(1, 100.0);
	qcpCPU->yAxis->setLabel(tr("Usage(%)"));

	qcpIO->xAxis->scaleRange(1.0, 30.0);
	qcpIO->xAxis->setLabel(tr("Time(second)"));
	qcpIO->yAxis->scaleRange(1, 100.0);
	qcpIO->yAxis->setLabel(tr("Usage(%)"));

	// per-VM stats table
	tableVMs->setHorizontalHeaderLabels( QString("VM Name,CPU Usage(%),Memory Usage(KB)").split(",") );

	/* setup signal-slot onnections across the GUI and worker(collect) threads*/
	connect(&m_memThread, SIGNAL( sigDataReady(QCustomPlot*) ), this, SLOT( updatePlot(QCustomPlot*) ));
	connect(&m_cpuThread, SIGNAL( sigDataReady(QCustomPlot*) ), this, SLOT( updatePlot(QCustomPlot*) ));
	connect(&m_ioThread, SIGNAL( sigDataReady(QCustomPlot*) ), this, SLOT( updatePlot(QCustomPlot*) ));

	/* direct(intra-thread) Qt GUI connections */
	connect(actionPause, SIGNAL( triggered() ), this, SLOT( onactionPause() ));
	connect(actionRestart, SIGNAL( triggered() ), this, SLOT( onactionRestart() ));
	connect(actionExit, SIGNAL( triggered() ), this, SLOT( onactionExit() ));

	collectThread::__init();
	perVMThread::__init();

	statusbar->showMessage(tr("Running..."));
}

VMMMonitorDialog::~VMMMonitorDialog()
{
	collectThread::__exit();
	perVMThread::__exit();
	for (int i=0; i<m_allVMThreads.size(); ++i) {
		m_allVMThreads[i]->halt();
		delete m_allVMThreads[i];
	}
}

void VMMMonitorDialog::closeEvent(QCloseEvent* event)
{
	m_memThread.halt();
	m_cpuThread.halt();
	m_ioThread.halt();
	for (int i=0; i<m_allVMThreads.size(); ++i) {
		m_allVMThreads[i]->halt();
	}
	event->accept();
}

/* Execute dialog and wait for completion: */
void VMMMonitorDialog::execute()
{
    if (exec() != QDialog::Accepted)
        return;
}

void VMMMonitorDialog::onactionPause()
{
	m_memThread.pause();
	m_cpuThread.pause();
	m_ioThread.pause();
	for (int i=0; i<m_allVMThreads.size(); ++i) {
		m_allVMThreads[i]->pause();
	}
	statusbar->showMessage(tr("Paused."));
}

void VMMMonitorDialog::onactionRestart()
{
	m_memThread.restart();
	m_cpuThread.restart();
	m_ioThread.restart();
	for (int i=0; i<m_allVMThreads.size(); ++i) {
		m_allVMThreads[i]->restart();
	}
	statusbar->showMessage(tr("Running..."));
}

void VMMMonitorDialog::onactionExit()
{
	m_memThread.halt();
	m_cpuThread.halt();
	m_ioThread.halt();
	for (int i=0; i<m_allVMThreads.size(); ++i) {
		m_allVMThreads[i]->halt();
	}
	emit close();
}

void VMMMonitorDialog::retranslateUi()
{
    /* Translate generated stuff: */
    Ui::VMMMonitorDialog::retranslateUi(this);
}

void VMMMonitorDialog::addGraph(QCustomPlot* plot, 
		const char* name, QCPGraph::LineStyle style, QCP::ScatterStyle markerStyle, int color)
{
	QPen pen;
	int i = color;
	pen.setColor(QColor(sin(i*1+1.2)*80+80, sin(i*0.3+0)*80+80, sin(i*0.3+1.5)*80+80));

	plot->addGraph();
	plot->graph()->setPen(pen);
	plot->graph()->setName(name);
	plot->graph()->setLineStyle(style);
	plot->graph()->setScatterStyle(markerStyle);
	plot->graph()->setScatterSize(5);

	plot->graph()->clearData();
	plot->graph()->rescaleAxes(true);

	// set blank axis lines:
	plot->xAxis->setTicks(true);
	plot->yAxis->setTicks(true);
	plot->xAxis->setTickLabels(true);
	plot->yAxis->setTickLabels(true);
	// make top right axes clones of bottom left axes:
	plot->setupFullAxesBox();
}

/*
void VMMMonitorDialog::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    QString text = tr("--- runtime performance metrics of VMM ---"
                      "Press and hold left mouse button to scroll.");
    QFontMetrics metrics = painter.fontMetrics();
    int textWidth = metrics.width(text);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 127));
    painter.drawRect((width() - textWidth) / 2 - 5, 0, textWidth + 10,
                     metrics.lineSpacing() + 5);
    painter.setPen(Qt::white);
    painter.drawText((width() - textWidth) / 2,
                     metrics.leading() + metrics.ascent(), text);
}
*/
//! [9]

//! [10]
void VMMMonitorDialog::resizeEvent(QResizeEvent * event)
{
	static bool bFirst = true;
	m_memThread.collect("host", "RAM/Usage/Used,RAM/VMM/Used,RAM/Usage/xxx", qcpMem);
	m_cpuThread.collect("host", "CPU/Load/User,CPU/Load/Kernel", qcpCPU);
	m_ioThread.collect("host", "Disk/ootfs/Load/Util,Net/wlan0/Load/Rx,Net/wlan0/Load/Tx", qcpIO);

	if ( !bFirst ) {
		startCollectAllVMs();

		QSize sz = event->size(), oldsz = event->oldSize();

		tabWidget->resize( 
				tabWidget->width() + sz.width() - oldsz.width(), 
				tabWidget->height() + sz.height() - oldsz.height() ); 

		tabWidget->currentWidget()->resize( 
				tabWidget->currentWidget()->width() + sz.width() - oldsz.width(), 
				tabWidget->currentWidget()->height() + sz.height() - oldsz.height() );

		// *0.5 because we need to equally heigthen two graphs, sharing the increment/decrement for the whole window
		qcpCPU->resize( qcpCPU->width() + sz.width() - oldsz.width(), qcpCPU->height() + (sz.height() - oldsz.height())*.5 );

		qcpMem->resize( qcpMem->width() + sz.width() - oldsz.width(), qcpMem->height() + (sz.height() - oldsz.height())*.5 );

		qcpMem->move( qcpMem->pos().x(), qcpMem->pos().y() + (sz.height() - oldsz.height())*.5 );
		labelMem->move( labelMem->pos().x(), labelMem->pos().y() + (sz.height() - oldsz.height())*.5 );

		qcpIO->resize( qcpIO->width() + sz.width() - oldsz.width(), qcpIO->height() + (sz.height() - oldsz.height())*1.0 );

		tableVMs->resize( tableVMs->width() + sz.width() - oldsz.width(), tableVMs->height() + (sz.height() - oldsz.height())*1.0 );
	}
	bFirst = false;
}
//! [10]

void VMMMonitorDialog::startCollectAllVMs()
{
	if ( m_allVMThreads.size() >= 1 ) {
		for (int i=0; i<m_allVMThreads.size(); ++i) {
			m_allVMThreads[i]->halt();
			do {
				sleep(1);
			} while ( m_allVMThreads[i]->isRunning() );
			delete m_allVMThreads[i];
		}

		m_allVMThreads.clear();
	}

	QStringList allVMs;
	perVMThread::getAllVMs( allVMs );


	for (int i=0; i<allVMs.size(); ++i) {
		m_allVMThreads.push_back(new perVMThread(NULL));

		connect(m_allVMThreads[i], SIGNAL( sigDataReady(const QString&, const QString&, const QString&) ), 
				this, SLOT( updateTable(const QString&, const QString&, const QString&) ));

		m_allVMThreads[i]->collect(allVMs[i].toStdString().c_str(), "CPU/Load/Kernel,RAM/Usage/Used");
	}
}

//! [16]
void VMMMonitorDialog::updatePlot(QCustomPlot* pplot)
{
	pplot->replot();
    update();
}

//! [16]
//
void VMMMonitorDialog::updateTable(const QString& vmName, const QString& cpuUsage, const QString& memUsage)
{
	QList<QTableWidgetItem*> possibleItems = tableVMs->findItems(vmName, Qt::MatchContains);
	int targetRow = 0;
	if ( possibleItems.size() != 1 ) {
		// DEBUG
		/*
		printf("when trying to update the perVM-stats table, VM name[%s] does not match any in the table, so added\n", 
				vmName.toStdString().c_str());
		*/
		targetRow = tableVMs->rowCount();
		tableVMs->insertRow(targetRow);
		tableVMs->setItem(targetRow, 0, new QTableWidgetItem(vmName));
	}
	else {
		targetRow = possibleItems[0]->row();
	}
	tableVMs->setItem(targetRow, 1, new QTableWidgetItem(cpuUsage));
	tableVMs->setItem(targetRow, 2, new QTableWidgetItem(memUsage));

	update();
}

/* vim set ts=8 sts=8 tw=80 */

