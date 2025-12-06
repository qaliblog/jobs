#include "network_scanner_widget.h"
#include <QNetworkInterface>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QThread>
#include <QTimer>

NetworkScannerWidget::NetworkScannerWidget(QWidget *parent)
    : QWidget(parent)
    , isScanning(false)
{
    setupUI();
}

NetworkScannerWidget::~NetworkScannerWidget()
{
    if (scanTimer) {
        scanTimer->stop();
        delete scanTimer;
    }
}

void NetworkScannerWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Control buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    scanButton = new QPushButton("Scan Network", this);
    stopButton = new QPushButton("Stop", this);
    refreshButton = new QPushButton("Refresh", this);
    recruitButton = new QPushButton("Recruit Selected", this);
    workForButton = new QPushButton("Work For Selected", this);
    
    stopButton->setEnabled(false);
    recruitButton->setEnabled(false);
    workForButton->setEnabled(false);
    
    buttonLayout->addWidget(scanButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(recruitButton);
    buttonLayout->addWidget(workForButton);
    
    // Status label
    statusLabel = new QLabel("Ready to scan", this);
    
    // Progress bar
    scanProgress = new QProgressBar(this);
    scanProgress->setRange(0, 0); // Indeterminate
    scanProgress->setVisible(false);
    
    // Device list
    deviceList = new QListWidget(this);
    deviceList->setAlternatingRowColors(true);
    
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addWidget(scanProgress);
    mainLayout->addWidget(deviceList);
    
    // Timer for scan progress
    scanTimer = new QTimer(this);
    
    // Connect signals
    connect(scanButton, &QPushButton::clicked, this, &NetworkScannerWidget::onScanClicked);
    connect(stopButton, &QPushButton::clicked, this, &NetworkScannerWidget::onStopClicked);
    connect(refreshButton, &QPushButton::clicked, this, &NetworkScannerWidget::onRefreshClicked);
    connect(recruitButton, &QPushButton::clicked, this, &NetworkScannerWidget::onRecruitClicked);
    connect(workForButton, &QPushButton::clicked, this, &NetworkScannerWidget::onWorkForClicked);
    connect(deviceList, &QListWidget::itemDoubleClicked, this, &NetworkScannerWidget::onDeviceDoubleClicked);
    connect(deviceList, &QListWidget::itemSelectionChanged, [this]() {
        bool hasSelection = deviceList->currentItem() != nullptr;
        recruitButton->setEnabled(hasSelection);
        workForButton->setEnabled(hasSelection);
    });
}

void NetworkScannerWidget::onScanClicked()
{
    scanNetwork();
}

void NetworkScannerWidget::onStopClicked()
{
    isScanning = false;
    scanButton->setEnabled(true);
    stopButton->setEnabled(false);
    scanProgress->setVisible(false);
    scanTimer->stop();
    statusLabel->setText("Scan stopped");
}

void NetworkScannerWidget::onRefreshClicked()
{
    deviceList->clear();
    discoveredDevices.clear();
    scanNetwork();
}

void NetworkScannerWidget::onDeviceDoubleClicked(QListWidgetItem* /* item */)
{
    DiscoveredDevice device = getSelectedDevice();
    if (!device.id.isEmpty()) {
        emit workRequested(device);
    }
}

void NetworkScannerWidget::onRecruitClicked()
{
    DiscoveredDevice device = getSelectedDevice();
    if (!device.id.isEmpty()) {
        emit recruitRequested(device);
    }
}

void NetworkScannerWidget::onWorkForClicked()
{
    DiscoveredDevice device = getSelectedDevice();
    if (!device.id.isEmpty()) {
        emit workRequested(device);
    }
}

void NetworkScannerWidget::updateScanProgress()
{
    statusLabel->setText(QString("Scanning... Found %1 devices").arg(discoveredDevices.size()));
}

void NetworkScannerWidget::scanNetwork()
{
    isScanning = true;
    scanButton->setEnabled(false);
    stopButton->setEnabled(true);
    scanProgress->setVisible(true);
    deviceList->clear();
    discoveredDevices.clear();
    statusLabel->setText("Scanning network...");
    
    scanTimer->start(1000);
    connect(scanTimer, &QTimer::timeout, this, &NetworkScannerWidget::updateScanProgress);
    
    // Get local network prefix
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    QStringList networkPrefixes;
    
    for (const QHostAddress &address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && 
            !address.isLoopback()) {
            QString ip = address.toString();
            int lastDot = ip.lastIndexOf('.');
            if (lastDot != -1) {
                networkPrefixes.append(ip.left(lastDot + 1));
            }
        }
    }
    
    if (networkPrefixes.isEmpty()) {
        networkPrefixes << "192.168.1." << "192.168.0." << "10.0.0.";
    }
    
    // Scan network in background thread (simplified - should use QThread)
    QTimer::singleShot(0, [this, networkPrefixes]() {
        for (const QString &prefix : networkPrefixes) {
            if (!isScanning) break;
            
            for (int i = 1; i < 255; ++i) {
                if (!isScanning) break;
                
                QString ip = prefix + QString::number(i);
                int port = 8080;
                
                QTcpSocket *socket = new QTcpSocket(this);
                socket->connectToHost(ip, port);
                
                if (socket->waitForConnected(200)) {
                    QString request = QString("GET /api/discover HTTP/1.1\r\nHost: %1:%2\r\n\r\n")
                                     .arg(ip).arg(port);
                    socket->write(request.toUtf8());
                    
                    if (socket->waitForReadyRead(500)) {
                        QByteArray response = socket->readAll();
                        parseDiscoveryResponse(response, ip, port);
                    }
                }
                
                socket->deleteLater();
                QThread::msleep(10); // Small delay
            }
        }
        
        isScanning = false;
        scanButton->setEnabled(true);
        stopButton->setEnabled(false);
        scanProgress->setVisible(false);
        scanTimer->stop();
        statusLabel->setText(QString("Scan complete. Found %1 devices").arg(discoveredDevices.size()));
    });
}

void NetworkScannerWidget::parseDiscoveryResponse(const QByteArray& response, const QString& address, int port)
{
    int jsonStart = response.indexOf('{');
    if (jsonStart == -1) return;
    
    QByteArray jsonData = response.mid(jsonStart);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
    
    if (error.error != QJsonParseError::NoError) return;
    
    QJsonObject obj = doc.object();
    
    DiscoveredDevice device;
    device.id = obj["id"].toString();
    device.name = obj["name"].toString();
    device.address = address;
    device.port = port;
    device.type = obj["type"].toString();
    device.isServer = obj["isServer"].toBool();
    device.isWorker = obj["isWorker"].toBool();
    
    addDiscoveredDevice(device);
}

void NetworkScannerWidget::addDiscoveredDevice(const DiscoveredDevice& device)
{
    // Check if already exists
    for (const DiscoveredDevice &existing : discoveredDevices) {
        if (existing.address == device.address && existing.port == device.port) {
            return;
        }
    }
    
    discoveredDevices.append(device);
    
    QString displayText = QString("%1 (%2) @ %3:%4")
                         .arg(device.name)
                         .arg(device.type)
                         .arg(device.address)
                         .arg(device.port);
    
    if (device.isServer) displayText += " [Server]";
    if (device.isWorker) displayText += " [Worker]";
    
    QListWidgetItem *item = new QListWidgetItem(displayText, deviceList);
    item->setData(Qt::UserRole, QVariant::fromValue(device));
}

DiscoveredDevice NetworkScannerWidget::getSelectedDevice()
{
    QListWidgetItem *item = deviceList->currentItem();
    if (item) {
        return item->data(Qt::UserRole).value<DiscoveredDevice>();
    }
    return DiscoveredDevice();
}

