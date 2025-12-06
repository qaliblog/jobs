#ifndef WORKER_MANAGER_H
#define WORKER_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <chrono>
#include <memory>
#include "task_queue.h"

enum class WorkerType {
    ANDROID,
    LINUX,
    WINDOWS,
    UNKNOWN
};

struct WorkerInfo {
    std::string id;
    std::string address;
    int port;
    WorkerType type;
    std::chrono::steady_clock::time_point lastHeartbeat;
    bool isActive;
    int cpuCores;
    double cpuUsage;
    long long availableMemory;
    bool hasGpu;
    int activeTasks;
    int completedTasks;
    
    WorkerInfo() : port(0), type(WorkerType::UNKNOWN), isActive(false),
                   cpuCores(0), cpuUsage(0.0), availableMemory(0),
                   hasGpu(false), activeTasks(0), completedTasks(0) {}
};

class WorkerManager {
public:
    WorkerManager();
    ~WorkerManager();
    
    // Worker registration
    std::string registerWorker(const std::string& workerType, 
                              const std::string& address, int port,
                              const std::string& capabilities);
    bool unregisterWorker(const std::string& workerId);
    bool updateWorkerHeartbeat(const std::string& workerId);
    
    // Worker selection
    std::string selectBestWorker(const Task& task);
    std::vector<std::string> getAvailableWorkers();
    WorkerInfo getWorkerInfo(const std::string& workerId);
    
    // Worker management
    void updateWorkerResources(const std::string& workerId, 
                              double cpuUsage, long long availableMemory);
    void incrementWorkerTasks(const std::string& workerId);
    void decrementWorkerTasks(const std::string& workerId);
    void incrementWorkerCompleted(const std::string& workerId);
    
    // Health monitoring
    void cleanupInactiveWorkers(int timeoutSeconds = 30);
    std::vector<WorkerInfo> getAllWorkers();
    int getActiveWorkerCount() const;
    
private:
    std::map<std::string, WorkerInfo> workers_;
    mutable std::mutex mutex_;
    int nextWorkerId_;
    
    WorkerType parseWorkerType(const std::string& type);
    double calculateWorkerScore(const WorkerInfo& worker, const Task& task);
    std::string generateWorkerId();
};

#endif // WORKER_MANAGER_H

