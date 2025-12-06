#ifndef RESOURCE_MONITOR_H
#define RESOURCE_MONITOR_H

struct WorkerResources {
    double cpuUsage;
    long long availableMemory;
    int cpuCores;
    
    WorkerResources() : cpuUsage(0.0), availableMemory(0), cpuCores(0) {}
};

class ResourceMonitor {
public:
    ResourceMonitor();
    WorkerResources getCurrentResources();
    std::string toJson() const;
    
private:
    WorkerResources resources_;
    void updateResources();
    double getCpuUsage();
    long long getAvailableMemory();
    int getCpuCores();
};

#endif // RESOURCE_MONITOR_H

