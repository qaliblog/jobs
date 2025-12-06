#include "resource_monitor_widget.h"
#include <QGroupBox>
#include <QGridLayout>
#include <QThread>
#include <QTimer>
#include <sys/sysinfo.h>
#include <fstream>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#endif

ResourceMonitorWidget::ResourceMonitorWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &ResourceMonitorWidget::updateResources);
    updateTimer->start(1000); // Update every second
    
    updateResources(); // Initial update
}

ResourceMonitorWidget::~ResourceMonitorWidget()
{
    if (updateTimer) {
        updateTimer->stop();
    }
}

void ResourceMonitorWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QGroupBox *resourceGroup = new QGroupBox("System Resources", this);
    QGridLayout *gridLayout = new QGridLayout(resourceGroup);
    
    // CPU
    gridLayout->addWidget(new QLabel("CPU Usage:", this), 0, 0);
    cpuLabel = new QLabel("0%", this);
    cpuProgress = new QProgressBar(this);
    cpuProgress->setRange(0, 100);
    cpuProgress->setTextVisible(true);
    gridLayout->addWidget(cpuLabel, 0, 1);
    gridLayout->addWidget(cpuProgress, 0, 2);
    
    // Memory
    gridLayout->addWidget(new QLabel("Memory Usage:", this), 1, 0);
    memoryLabel = new QLabel("0 MB", this);
    memoryProgress = new QProgressBar(this);
    memoryProgress->setRange(0, 100);
    memoryProgress->setTextVisible(true);
    gridLayout->addWidget(memoryLabel, 1, 1);
    gridLayout->addWidget(memoryProgress, 1, 2);
    
    // CPU Cores
    gridLayout->addWidget(new QLabel("CPU Cores:", this), 2, 0);
    coresLabel = new QLabel("0", this);
    gridLayout->addWidget(coresLabel, 2, 1, 1, 2);
    
    mainLayout->addWidget(resourceGroup);
    mainLayout->addStretch();
}

void ResourceMonitorWidget::updateResources()
{
    double cpuUsage = getCpuUsage();
    long long availableMemory = getAvailableMemory();
    int cpuCores = getCpuCores();
    
    cpuLabel->setText(QString("%1%").arg(cpuUsage, 0, 'f', 1));
    cpuProgress->setValue(static_cast<int>(cpuUsage));
    
    memoryLabel->setText(QString("%1 MB available").arg(availableMemory));
    memoryProgress->setValue(100 - (availableMemory * 100 / (availableMemory + 1024))); // Simplified
    
    coresLabel->setText(QString::number(cpuCores));
}

double ResourceMonitorWidget::getCpuUsage()
{
    static long long lastIdle = 0, lastTotal = 0;
    
    std::ifstream statFile("/proc/stat");
    if (!statFile.is_open()) return 0.0;
    
    std::string line;
    std::getline(statFile, line);
    statFile.close();
    
    std::istringstream iss(line);
    std::string cpu;
    long long user, nice, system, idle, iowait, irq, softirq;
    
    iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
    
    long long totalIdle = idle + iowait;
    long long totalNonIdle = user + nice + system + irq + softirq;
    long long total = totalIdle + totalNonIdle;
    
    double cpuPercent = 0.0;
    if (lastTotal > 0) {
        long long totalDelta = total - lastTotal;
        long long idleDelta = totalIdle - lastIdle;
        cpuPercent = 100.0 * (totalDelta - idleDelta) / totalDelta;
    }
    
    lastIdle = totalIdle;
    lastTotal = total;
    
    return std::max(0.0, std::min(100.0, cpuPercent));
}

long long ResourceMonitorWidget::getAvailableMemory()
{
    struct sysinfo info;
    if (sysinfo(&info) != 0) return 0;
    return (info.freeram * info.mem_unit) / (1024 * 1024); // MB
}

int ResourceMonitorWidget::getCpuCores()
{
    return QThread::idealThreadCount();
}

