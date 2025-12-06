#include "resource_monitor.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <algorithm>

#ifdef __linux__
#include <sys/resource.h>
#endif

ResourceMonitor::ResourceMonitor() {
    updateResources();
}

SystemResources ResourceMonitor::getCurrentResources() {
    updateResources();
    return resources_;
}

std::string ResourceMonitor::toJson() const {
    std::ostringstream json;
    json << "{\"cpuUsage\":" << resources_.cpuUsage
         << ",\"memoryUsage\":" << resources_.memoryUsage
         << ",\"gpuUsage\":" << resources_.gpuUsage
         << ",\"availableMemory\":" << resources_.availableMemory
         << ",\"totalMemory\":" << resources_.totalMemory
         << ",\"cpuCores\":" << resources_.cpuCores << "}";
    return json.str();
}

void ResourceMonitor::updateResources() {
    resources_.cpuUsage = getCpuUsage();
    resources_.memoryUsage = getMemoryUsage();
    resources_.gpuUsage = getGpuUsage();
    resources_.availableMemory = getAvailableMemory();
    resources_.totalMemory = getTotalMemory();
    resources_.cpuCores = getCpuCores();
}

double ResourceMonitor::getCpuUsage() {
    static long long last_idle = 0, last_total = 0;
    
    std::ifstream stat_file("/proc/stat");
    if (!stat_file.is_open()) return 0.0;
    
    std::string line;
    std::getline(stat_file, line);
    stat_file.close();
    
    std::istringstream iss(line);
    std::string cpu;
    long long user, nice, system, idle, iowait, irq, softirq;
    
    iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;
    
    long long total_idle = idle + iowait;
    long long total_non_idle = user + nice + system + irq + softirq;
    long long total = total_idle + total_non_idle;
    
    double cpu_percent = 0.0;
    if (last_total > 0) {
        long long total_delta = total - last_total;
        long long idle_delta = total_idle - last_idle;
        cpu_percent = 100.0 * (total_delta - idle_delta) / total_delta;
    }
    
    last_idle = total_idle;
    last_total = total;
    
    return std::max(0.0, std::min(100.0, cpu_percent));
}

double ResourceMonitor::getMemoryUsage() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) return 0.0;
    
    long long total_mem = info.totalram * info.mem_unit;
    long long free_mem = info.freeram * info.mem_unit;
    long long used_mem = total_mem - free_mem;
    
    if (total_mem > 0) {
        return 100.0 * used_mem / total_mem;
    }
    
    return 0.0;
}

double ResourceMonitor::getGpuUsage() {
    // GPU monitoring is platform-specific
    // This is a placeholder - in production, use vendor-specific APIs
    // (NVIDIA nvidia-smi, AMD rocm-smi, Intel intel_gpu_top, etc.)
    return 0.0; // Placeholder
}

long long ResourceMonitor::getAvailableMemory() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) return 0;
    return (info.freeram * info.mem_unit) / (1024 * 1024); // MB
}

long long ResourceMonitor::getTotalMemory() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) return 0;
    return (info.totalram * info.mem_unit) / (1024 * 1024); // MB
}

int ResourceMonitor::getCpuCores() {
    return std::thread::hardware_concurrency();
}

