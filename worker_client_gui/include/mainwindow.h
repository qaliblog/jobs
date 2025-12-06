#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include "qr_code_widget.h"
#include "connection_widget.h"
#include "resource_monitor_widget.h"
#include "task_status_widget.h"
#include "requests_widget.h"

QT_BEGIN_NAMESPACE
class QTabWidget;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAbout();
    void onQuit();
    void onServerModeToggled(bool enabled);

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    
    QTabWidget *tabWidget;
    QRCodeWidget *qrCodeWidget;
    ConnectionWidget *connectionWidget;
    ResourceMonitorWidget *resourceMonitorWidget;
    TaskStatusWidget *taskStatusWidget;
    RequestsWidget *requestsWidget;
    
    QAction *serverModeAction;
    bool isServerMode;
};

#endif // MAINWINDOW_H

