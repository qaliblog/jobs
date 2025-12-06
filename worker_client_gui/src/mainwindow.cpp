#include "mainwindow.h"
#include "qr_code_widget.h"
#include "requests_widget.h"
#include <QMessageBox>
#include <QApplication>
#include <QIcon>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , isServerMode(false)
{
    setupUI();
    setupMenuBar();
    setupStatusBar();
    
    setWindowTitle("Jobs Worker - Distributed Computing");
    setMinimumSize(800, 600);
    resize(1000, 700);
    
    // Set window icon from resources
    QIcon windowIcon(":/icons/icon-512.png");
    if (windowIcon.isNull()) {
        // Fallback to file system
        if (QFile::exists("../icons/web/icon-512.png")) {
            windowIcon.addFile("../icons/web/icon-512.png");
        } else if (QFile::exists("icons/web/icon-512.png")) {
            windowIcon.addFile("icons/web/icon-512.png");
        } else if (QFile::exists("../../icons/web/icon-512.png")) {
            windowIcon.addFile("../../icons/web/icon-512.png");
        }
    }
    if (!windowIcon.isNull()) {
        setWindowIcon(windowIcon);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // Create tab widget
    tabWidget = new QTabWidget(this);
    
    // QR Code Connection Tab
    qrCodeWidget = new QRCodeWidget(this);
    tabWidget->addTab(qrCodeWidget, "QR Code");
    
    // Connection Tab
    connectionWidget = new ConnectionWidget(this);
    tabWidget->addTab(connectionWidget, "Connection");
    
    // Resource Monitor Tab
    resourceMonitorWidget = new ResourceMonitorWidget(this);
    tabWidget->addTab(resourceMonitorWidget, "Resources");
    
    // Task Status Tab
    taskStatusWidget = new TaskStatusWidget(this);
    tabWidget->addTab(taskStatusWidget, "Tasks");
    
    // Requests Tab
    requestsWidget = new RequestsWidget(this);
    tabWidget->addTab(requestsWidget, "Requests");
    
    mainLayout->addWidget(tabWidget);
    
    // Connect signals
    connect(connectionWidget, &ConnectionWidget::serverModeToggled,
            this, &MainWindow::onServerModeToggled);
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *quitAction = fileMenu->addAction("Quit");
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &MainWindow::onQuit);
    
    // View menu
    QMenu *viewMenu = menuBar->addMenu("View");
    serverModeAction = viewMenu->addAction("Server Mode");
    serverModeAction->setCheckable(true);
    connect(serverModeAction, &QAction::toggled, this, &MainWindow::onServerModeToggled);
    
    // Help menu
    QMenu *helpMenu = menuBar->addMenu("Help");
    QAction *aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready");
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "About Jobs Worker",
        "Jobs Worker - Distributed Computing Platform\n\n"
        "Version 1.0\n\n"
        "A peer-to-peer distributed computing system that enables "
        "seamless collaboration between devices for compute-intensive tasks.");
}

void MainWindow::onQuit()
{
    QApplication::quit();
}

void MainWindow::onServerModeToggled(bool enabled)
{
    isServerMode = enabled;
    statusBar()->showMessage(enabled ? "Server mode enabled" : "Server mode disabled");
}

