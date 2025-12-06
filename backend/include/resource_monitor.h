#ifndef RESOURCE_MONITOR_H
#define RESOURCE_MONITOR_H

#include <string>

struct SystemResources {
    double cpuUsage;
    double memoryUsage;
    double gpuUsage;
    long long availableMemory;
    long long totalMemory;
    int cpuCores;
    
    SystemResources() : cpuUsage(0.0), memoryUsage(0.0), gpuUsage(0.0),
                       availableMemory(0), totalMemory(0), cpuCores(0) {}
};

class ResourceMonitor {
public:
    ResourceMonitor();
    SystemResources getCurrentResources();
    std::string toJson() const;
    
private:
    SystemResources resources_;
    void updateResources();
    double getCpuUsage();
    double getMemoryUsage();
    double getGpuUsage();
    long long getAvailableMemory();
    long long getTotalMemory();
    int getCpuCores();
};

#endif // RESOURCE_MONITOR_H

