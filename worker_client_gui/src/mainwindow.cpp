#include "mainwindow.h"
#include <QMessageBox>
#include <QApplication>

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
    
    // Network Scanner Tab
    networkScannerWidget = new NetworkScannerWidget(this);
    tabWidget->addTab(networkScannerWidget, "Network Discovery");
    
    // Connection Tab
    connectionWidget = new ConnectionWidget(this);
    tabWidget->addTab(connectionWidget, "Connection");
    
    // Resource Monitor Tab
    resourceMonitorWidget = new ResourceMonitorWidget(this);
    tabWidget->addTab(resourceMonitorWidget, "Resources");
    
    // Task Status Tab
    taskStatusWidget = new TaskStatusWidget(this);
    tabWidget->addTab(taskStatusWidget, "Tasks");
    
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

