#include "resource_monitor.h"
#include <fstream>
#include <sstream>
#include <thread>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <psapi.h>
#else
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

ResourceMonitor::ResourceMonitor() {
    updateResources();
}

WorkerResources ResourceMonitor::getCurrentResources() {
    updateResources();
    return resources_;
}

std::string ResourceMonitor::toJson() const {
    std::ostringstream json;
    json << "{\"cpuUsage\":" << resources_.cpuUsage
         << ",\"availableMemory\":" << resources_.availableMemory
         << ",\"cpuCores\":" << resources_.cpuCores << "}";
    return json.str();
}

void ResourceMonitor::updateResources() {
    resources_.cpuUsage = getCpuUsage();
    resources_.availableMemory = getAvailableMemory();
    resources_.cpuCores = getCpuCores();
}

double ResourceMonitor::getCpuUsage() {
    #ifdef _WIN32
    // Windows CPU usage calculation
    static FILETIME lastIdle, lastKernel, lastUser;
    FILETIME idle, kernel, user;
    
    if (GetSystemTimes(&idle, &kernel, &user)) {
        ULARGE_INTEGER idleTime, kernelTime, userTime;
        idleTime.LowPart = idle.dwLowDateTime;
        idleTime.HighPart = idle.dwHighDateTime;
        
        ULARGE_INTEGER lastIdleTime;
        lastIdleTime.LowPart = lastIdle.dwLowDateTime;
        lastIdleTime.HighPart = lastIdle.dwHighDateTime;
        
        ULARGE_INTEGER kernelTimeTotal, userTimeTotal;
        kernelTimeTotal.LowPart = kernel.dwLowDateTime;
        kernelTimeTotal.HighPart = kernel.dwHighDateTime;
        userTimeTotal.LowPart = user.dwLowDateTime;
        userTimeTotal.HighPart = user.dwHighDateTime;
        
        ULARGE_INTEGER totalTime;
        totalTime.QuadPart = (kernelTimeTotal.QuadPart + userTimeTotal.QuadPart) -
                            (lastIdleTime.QuadPart + idleTime.QuadPart);
        
        if (totalTime.QuadPart > 0) {
            double cpuPercent = 100.0 * (1.0 - (idleTime.QuadPart - lastIdleTime.QuadPart) / (double)totalTime.QuadPart);
            lastIdle = idle;
            lastKernel = kernel;
            lastUser = user;
            return cpuPercent;
        }
    }
    return 0.0;
    #else
    // Linux CPU usage
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
    #endif
}

long long ResourceMonitor::getAvailableMemory() {
    #ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return memInfo.ullAvailPhys / (1024 * 1024); // MB
    #else
    struct sysinfo info;
    if (sysinfo(&info) != 0) return 0;
    return (info.freeram * info.mem_unit) / (1024 * 1024); // MB
    #endif
}

int ResourceMonitor::getCpuCores() {
    return std::thread::hardware_concurrency();
}

