#include "resource_monitor.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/sysinfo.h>
#include <unistd.h>
#ifdef __linux__
#include <sys/resource.h>
#endif
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
#ifdef _WIN32
    // Windows CPU usage - simplified version
    static FILETIME last_idle, last_kernel, last_user;
    FILETIME idle, kernel, user;
    
    if (GetSystemTimes(&idle, &kernel, &user)) {
        ULARGE_INTEGER idle_time, kernel_time, user_time;
        idle_time.LowPart = idle.dwLowDateTime;
        idle_time.HighPart = idle.dwHighDateTime;
        kernel_time.LowPart = kernel.dwLowDateTime;
        kernel_time.HighPart = kernel.dwHighDateTime;
        user_time.LowPart = user.dwLowDateTime;
        user_time.HighPart = user.dwHighDateTime;
        
        ULARGE_INTEGER last_idle_time, last_total_time;
        last_idle_time.LowPart = last_idle.dwLowDateTime;
        last_idle_time.HighPart = last_idle.dwHighDateTime;
        last_total_time.QuadPart = (ULARGE_INTEGER{{last_kernel.dwLowDateTime, last_kernel.dwHighDateTime}}).QuadPart +
                                   (ULARGE_INTEGER{{last_user.dwLowDateTime, last_user.dwHighDateTime}}).QuadPart;
        
        ULARGE_INTEGER total_time;
        total_time.QuadPart = kernel_time.QuadPart + user_time.QuadPart;
        
        double cpu_percent = 0.0;
        if (last_total_time.QuadPart > 0) {
            ULONGLONG total_delta = total_time.QuadPart - last_total_time.QuadPart;
            ULONGLONG idle_delta = idle_time.QuadPart - last_idle_time.QuadPart;
            cpu_percent = 100.0 * (1.0 - (double)idle_delta / total_delta);
        }
        
        last_idle = idle;
        last_kernel = kernel;
        last_user = user;
        
        return std::max(0.0, std::min(100.0, cpu_percent));
    }
    return 0.0;
#else
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
#endif
}

double ResourceMonitor::getMemoryUsage() {
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        DWORDLONG total_mem = memInfo.ullTotalPhys;
        DWORDLONG free_mem = memInfo.ullAvailPhys;
        DWORDLONG used_mem = total_mem - free_mem;
        
        if (total_mem > 0) {
            return 100.0 * used_mem / total_mem;
        }
    }
    return 0.0;
#else
    struct sysinfo info;
    if (sysinfo(&info) != 0) return 0.0;
    
    long long total_mem = info.totalram * info.mem_unit;
    long long free_mem = info.freeram * info.mem_unit;
    long long used_mem = total_mem - free_mem;
    
    if (total_mem > 0) {
        return 100.0 * used_mem / total_mem;
    }
    
    return 0.0;
#endif
}

double ResourceMonitor::getGpuUsage() {
    // GPU monitoring is platform-specific
    // This is a placeholder - in production, use vendor-specific APIs
    // (NVIDIA nvidia-smi, AMD rocm-smi, Intel intel_gpu_top, etc.)
    return 0.0; // Placeholder
}

long long ResourceMonitor::getAvailableMemory() {
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return memInfo.ullAvailPhys / (1024 * 1024); // MB
    }
    return 0;
#else
    struct sysinfo info;
    if (sysinfo(&info) != 0) return 0;
    return (info.freeram * info.mem_unit) / (1024 * 1024); // MB
#endif
}

long long ResourceMonitor::getTotalMemory() {
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return memInfo.ullTotalPhys / (1024 * 1024); // MB
    }
    return 0;
#else
    struct sysinfo info;
    if (sysinfo(&info) != 0) return 0;
    return (info.totalram * info.mem_unit) / (1024 * 1024); // MB
#endif
}

int ResourceMonitor::getCpuCores() {
    return std::thread::hardware_concurrency();
}

