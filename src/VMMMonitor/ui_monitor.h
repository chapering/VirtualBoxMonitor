/********************************************************************************
** Form generated from reading UI file 'monitor.ui'
**
** Created: Sat Dec 8 22:23:02 2012
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MONITOR_H
#define UI_MONITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>
#include <qcustomplot.h>

QT_BEGIN_NAMESPACE

class Ui_VMMMonitorDialog
{
public:
    QAction *actionExit;
    QAction *actionRestart;
    QAction *actionPause;
    QWidget *centralwidget;
    QTabWidget *tabWidget;
    QWidget *tabCPUMem;
    QLabel *labelCPU;
    QLabel *labelMem;
    QCustomPlot *qcpCPU;
    QCustomPlot *qcpMem;
    QWidget *tabIO;
    QCustomPlot *qcpIO;
    QLabel *labelIO;
    QWidget *tabVMs;
    QTableWidget *tableVMs;
    QLabel *labelVMs;
    QMenuBar *menubar;
    QMenu *menuMain;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *VMMMonitorDialog)
    {
        if (VMMMonitorDialog->objectName().isEmpty())
            VMMMonitorDialog->setObjectName(QString::fromUtf8("VMMMonitorDialog"));
        VMMMonitorDialog->resize(734, 584);
        actionExit = new QAction(VMMMonitorDialog);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionRestart = new QAction(VMMMonitorDialog);
        actionRestart->setObjectName(QString::fromUtf8("actionRestart"));
        actionPause = new QAction(VMMMonitorDialog);
        actionPause->setObjectName(QString::fromUtf8("actionPause"));
        centralwidget = new QWidget(VMMMonitorDialog);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(0, 0, 731, 531));
        tabCPUMem = new QWidget();
        tabCPUMem->setObjectName(QString::fromUtf8("tabCPUMem"));
        labelCPU = new QLabel(tabCPUMem);
        labelCPU->setObjectName(QString::fromUtf8("labelCPU"));
        labelCPU->setGeometry(QRect(10, -10, 711, 31));
        labelCPU->setAlignment(Qt::AlignCenter);
        labelMem = new QLabel(tabCPUMem);
        labelMem->setObjectName(QString::fromUtf8("labelMem"));
        labelMem->setGeometry(QRect(10, 260, 711, 21));
        labelMem->setAlignment(Qt::AlignCenter);
        qcpCPU = new QCustomPlot(tabCPUMem);
        qcpCPU->setObjectName(QString::fromUtf8("qcpCPU"));
        qcpCPU->setGeometry(QRect(10, 20, 711, 241));
        qcpMem = new QCustomPlot(tabCPUMem);
        qcpMem->setObjectName(QString::fromUtf8("qcpMem"));
        qcpMem->setGeometry(QRect(10, 280, 711, 211));
        tabWidget->addTab(tabCPUMem, QString());
        tabIO = new QWidget();
        tabIO->setObjectName(QString::fromUtf8("tabIO"));
        qcpIO = new QCustomPlot(tabIO);
        qcpIO->setObjectName(QString::fromUtf8("qcpIO"));
        qcpIO->setGeometry(QRect(0, 20, 721, 471));
        labelIO = new QLabel(tabIO);
        labelIO->setObjectName(QString::fromUtf8("labelIO"));
        labelIO->setGeometry(QRect(0, -10, 711, 31));
        labelIO->setAlignment(Qt::AlignCenter);
        tabWidget->addTab(tabIO, QString());
        tabVMs = new QWidget();
        tabVMs->setObjectName(QString::fromUtf8("tabVMs"));
        tableVMs = new QTableWidget(tabVMs);
        if (tableVMs->columnCount() < 3)
            tableVMs->setColumnCount(3);
        tableVMs->setObjectName(QString::fromUtf8("tableVMs"));
        tableVMs->setGeometry(QRect(0, 20, 731, 471));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(tableVMs->sizePolicy().hasHeightForWidth());
        tableVMs->setSizePolicy(sizePolicy);
        tableVMs->viewport()->setProperty("cursor", QVariant(QCursor(Qt::ArrowCursor)));
        tableVMs->setLineWidth(2);
        tableVMs->setMidLineWidth(1);
        tableVMs->setAlternatingRowColors(true);
        tableVMs->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableVMs->setSortingEnabled(true);
        tableVMs->setColumnCount(3);
        tableVMs->horizontalHeader()->setCascadingSectionResizes(false);
        tableVMs->horizontalHeader()->setStretchLastSection(true);
        tableVMs->verticalHeader()->setCascadingSectionResizes(false);
        tableVMs->verticalHeader()->setDefaultSectionSize(16);
        tableVMs->verticalHeader()->setMinimumSectionSize(10);
        tableVMs->verticalHeader()->setProperty("showSortIndicator", QVariant(true));
        tableVMs->verticalHeader()->setStretchLastSection(false);
        labelVMs = new QLabel(tabVMs);
        labelVMs->setObjectName(QString::fromUtf8("labelVMs"));
        labelVMs->setGeometry(QRect(10, -10, 711, 31));
        labelVMs->setAlignment(Qt::AlignCenter);
        tabWidget->addTab(tabVMs, QString());
        VMMMonitorDialog->setCentralWidget(centralwidget);
        menubar = new QMenuBar(VMMMonitorDialog);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 734, 29));
        menuMain = new QMenu(menubar);
        menuMain->setObjectName(QString::fromUtf8("menuMain"));
        VMMMonitorDialog->setMenuBar(menubar);
        statusbar = new QStatusBar(VMMMonitorDialog);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        statusbar->setEnabled(true);
        VMMMonitorDialog->setStatusBar(statusbar);

        menubar->addAction(menuMain->menuAction());
        menuMain->addAction(actionPause);
        menuMain->addAction(actionRestart);
        menuMain->addSeparator();
        menuMain->addAction(actionExit);

        retranslateUi(VMMMonitorDialog);

        tabWidget->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(VMMMonitorDialog);
    } // setupUi

    void retranslateUi(QMainWindow *VMMMonitorDialog)
    {
        VMMMonitorDialog->setWindowTitle(QApplication::translate("VMMMonitorDialog", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("VMMMonitorDialog", "Exit", 0, QApplication::UnicodeUTF8));
        actionRestart->setText(QApplication::translate("VMMMonitorDialog", "Restart", 0, QApplication::UnicodeUTF8));
        actionPause->setText(QApplication::translate("VMMMonitorDialog", "Pause", 0, QApplication::UnicodeUTF8));
        labelCPU->setText(QApplication::translate("VMMMonitorDialog", "CPU Usage: VMM vs Whole Virtualizer", 0, QApplication::UnicodeUTF8));
        labelMem->setText(QApplication::translate("VMMMonitorDialog", "Memory Usage: VMM vs Whole Virtualizer", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabCPUMem), QApplication::translate("VMMMonitorDialog", "Tab 1", 0, QApplication::UnicodeUTF8));
        labelIO->setText(QApplication::translate("VMMMonitorDialog", "Disk/Net I/O Dynamics in VMM", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabIO), QApplication::translate("VMMMonitorDialog", "Page", 0, QApplication::UnicodeUTF8));
        labelVMs->setText(QApplication::translate("VMMMonitorDialog", "Per-VM Performance Statistics", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabVMs), QApplication::translate("VMMMonitorDialog", "Tab 2", 0, QApplication::UnicodeUTF8));
        menuMain->setTitle(QApplication::translate("VMMMonitorDialog", "Main Controls...", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class VMMMonitorDialog: public Ui_VMMMonitorDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MONITOR_H
