#ifndef RESOURCE_MONITOR_WIDGET_H
#define RESOURCE_MONITOR_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QGroupBox>

class ResourceMonitorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResourceMonitorWidget(QWidget *parent = nullptr);
    ~ResourceMonitorWidget();

private slots:
    void updateResources();

private:
    void setupUI();
    double getCpuUsage();
    long long getAvailableMemory();
    int getCpuCores();
    
    QLabel *cpuLabel;
    QLabel *memoryLabel;
    QLabel *coresLabel;
    QProgressBar *cpuProgress;
    QProgressBar *memoryProgress;
    QTimer *updateTimer;
};

#endif // RESOURCE_MONITOR_WIDGET_H

