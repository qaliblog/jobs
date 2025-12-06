#ifndef NETWORK_SCANNER_WIDGET_H
#define NETWORK_SCANNER_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>

struct DiscoveredDevice {
    QString id;
    QString name;
    QString address;
    int port;
    QString type;
    bool isServer;
    bool isWorker;
};

class NetworkScannerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkScannerWidget(QWidget *parent = nullptr);
    ~NetworkScannerWidget();

signals:
    void deviceSelected(const DiscoveredDevice& device);
    void recruitRequested(const DiscoveredDevice& device);
    void workRequested(const DiscoveredDevice& device);

private slots:
    void onScanClicked();
    void onStopClicked();
    void onRefreshClicked();
    void onDeviceDoubleClicked(QListWidgetItem* item);
    void onRecruitClicked();
    void onWorkForClicked();
    void updateScanProgress();

private:
    void setupUI();
    void scanNetwork();
    void addDiscoveredDevice(const DiscoveredDevice& device);
    DiscoveredDevice getSelectedDevice();
    void parseDiscoveryResponse(const QByteArray& response, const QString& address, int port);
    
    QPushButton *scanButton;
    QPushButton *stopButton;
    QPushButton *refreshButton;
    QPushButton *recruitButton;
    QPushButton *workForButton;
    QListWidget *deviceList;
    QLabel *statusLabel;
    QProgressBar *scanProgress;
    QTimer *scanTimer;
    
    bool isScanning;
    QList<DiscoveredDevice> discoveredDevices;
};

#endif // NETWORK_SCANNER_WIDGET_H

