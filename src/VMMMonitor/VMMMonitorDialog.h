/** @file
 *
 * VBox frontends: Qt4 GUI ("VirtualBox"):
 * VMMMonitorDialog class declaration
 */

/*
 * Copyright (C) 2012-2013 
 *
 * This file is part of the final project for gradOS Fall 2012
 */

#ifndef __VMMMonitorDialog_h__
#define __VMMMonitorDialog_h__
/* Qt includes */
#include <QtCore>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QList>
#include <QTableWidget>
#include <QTableWidgetItem>
//#include <QWidget>
//#include <QMainWindow>
#include <QCloseEvent>

/* Locals */
#include "QIMainDialog.h"
#include "QIWithRetranslateUI.h"
#include "ui_monitor.h"

#include "collectThread.h"
#include "perVMThread.h"

/* Base dialog class for both Global & VM settings which encapsulates most of their similar functionalities */
class VMMMonitorDialog : public QIWithRetranslateUI<QIMainDialog>, public Ui::VMMMonitorDialog
//class VMMMonitorDialog : public QMainWindow, public Ui::VMMMonitorDialog
{
    Q_OBJECT;

public:

    /* Settings Dialog Constructor/Destructor: */
    VMMMonitorDialog(QWidget *pParent);
   ~VMMMonitorDialog();

    /* Execute API: */
    void execute();

public slots:
	void onactionPause();
	void onactionRestart();
	void onactionExit();

protected:

    /* UI translator: */
    virtual void retranslateUi();

	/* create sub Graph in a plot */
	void addGraph(QCustomPlot* plot, const char* name, QCPGraph::LineStyle style, QCP::ScatterStyle markerStyle, int color=0);

	/* overload event handler for stopping metrics collection */
	void closeEvent(QCloseEvent* event);

    //void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

	// create a perVMThread for each of the registered VMs
	void startCollectAllVMs();

private slots:
	void updatePlot(QCustomPlot* pplot);
	void updateTable(const QString& vmName, const QString& cpuUsage, const QString& memUsage);

private:

	collectThread	m_memThread;
	collectThread	m_cpuThread;
	collectThread	m_ioThread;
	QList<perVMThread*> m_allVMThreads;
};

#endif // __VMMMonitorDialog_h__

