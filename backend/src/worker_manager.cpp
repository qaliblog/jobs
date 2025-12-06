#include "worker_manager.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>

WorkerManager::WorkerManager() : nextWorkerId_(1) {
}

WorkerManager::~WorkerManager() {
}

std::string WorkerManager::registerWorker(const std::string& workerType,
                                          const std::string& address, int port,
                                          const std::string& capabilities) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    WorkerInfo worker;
    worker.id = generateWorkerId();
    worker.address = address;
    worker.port = port;
    worker.type = parseWorkerType(workerType);
    worker.lastHeartbeat = std::chrono::steady_clock::now();
    worker.isActive = true;
    
    // Parse capabilities
    // Format: "cpuCores:8,availableMemory:16384,hasGpu:true"
    std::istringstream capStream(capabilities);
    std::string item;
    while (std::getline(capStream, item, ',')) {
        size_t colonPos = item.find(':');
        if (colonPos != std::string::npos) {
            std::string key = item.substr(0, colonPos);
            std::string value = item.substr(colonPos + 1);
            
            if (key == "cpuCores") {
                worker.cpuCores = std::stoi(value);
            } else if (key == "availableMemory") {
                worker.availableMemory = std::stoll(value);
            } else if (key == "hasGpu") {
                worker.hasGpu = (value == "true");
            }
        }
    }
    
    workers_[worker.id] = worker;
    
    std::cout << "Worker registered: " << worker.id 
              << " (" << workerType << " @ " << address << ":" << port << ")\n";
    
    return worker.id;
}

bool WorkerManager::unregisterWorker(const std::string& workerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = workers_.find(workerId);
    if (it != workers_.end()) {
        std::cout << "Worker unregistered: " << workerId << "\n";
        workers_.erase(it);
        return true;
    }
    
    return false;
}

bool WorkerManager::updateWorkerHeartbeat(const std::string& workerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = workers_.find(workerId);
    if (it != workers_.end()) {
        it->second.lastHeartbeat = std::chrono::steady_clock::now();
        it->second.isActive = true;
        return true;
    }
    
    return false;
}

std::string WorkerManager::selectBestWorker(const Task& task) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (workers_.empty()) {
        return ""; // No workers available, use local processing
    }
    
    std::string bestWorkerId;
    double bestScore = -1.0;
    
    for (const auto& pair : workers_) {
        const WorkerInfo& worker = pair.second;
        
        if (!worker.isActive) continue;
        
        double score = calculateWorkerScore(worker, task);
        if (score > bestScore) {
            bestScore = score;
            bestWorkerId = worker.id;
        }
    }
    
    return bestWorkerId;
}

std::vector<std::string> WorkerManager::getAvailableWorkers() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> available;
    for (const auto& pair : workers_) {
        if (pair.second.isActive) {
            available.push_back(pair.first);
        }
    }
    
    return available;
}

WorkerInfo WorkerManager::getWorkerInfo(const std::string& workerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = workers_.find(workerId);
    if (it != workers_.end()) {
        return it->second;
    }
    
    return WorkerInfo();
}

void WorkerManager::updateWorkerResources(const std::string& workerId,
                                         double cpuUsage, long long availableMemory) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = workers_.find(workerId);
    if (it != workers_.end()) {
        it->second.cpuUsage = cpuUsage;
        it->second.availableMemory = availableMemory;
    }
}

void WorkerManager::incrementWorkerTasks(const std::string& workerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = workers_.find(workerId);
    if (it != workers_.end()) {
        it->second.activeTasks++;
    }
}

void WorkerManager::decrementWorkerTasks(const std::string& workerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = workers_.find(workerId);
    if (it != workers_.end()) {
        if (it->second.activeTasks > 0) {
            it->second.activeTasks--;
        }
    }
}

void WorkerManager::incrementWorkerCompleted(const std::string& workerId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = workers_.find(workerId);
    if (it != workers_.end()) {
        it->second.completedTasks++;
    }
}

void WorkerManager::cleanupInactiveWorkers(int timeoutSeconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto timeout = std::chrono::seconds(timeoutSeconds);
    
    auto it = workers_.begin();
    while (it != workers_.end()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.lastHeartbeat);
        
        if (elapsed > timeout) {
            std::cout << "Worker timeout: " << it->first << "\n";
            it = workers_.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<WorkerInfo> WorkerManager::getAllWorkers() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<WorkerInfo> result;
    for (const auto& pair : workers_) {
        result.push_back(pair.second);
    }
    
    return result;
}

int WorkerManager::getActiveWorkerCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    int count = 0;
    for (const auto& pair : workers_) {
        if (pair.second.isActive) {
            count++;
        }
    }
    
    return count;
}

WorkerType WorkerManager::parseWorkerType(const std::string& type) {
    if (type == "android" || type == "Android") {
        return WorkerType::ANDROID;
    } else if (type == "linux" || type == "Linux") {
        return WorkerType::LINUX;
    } else if (type == "windows" || type == "Windows") {
        return WorkerType::WINDOWS;
    }
    return WorkerType::UNKNOWN;
}

double WorkerManager::calculateWorkerScore(const WorkerInfo& worker, const Task& task) {
    double score = 100.0;
    
    // Penalize high CPU usage
    score -= worker.cpuUsage * 0.5;
    
    // Penalize high active task count
    score -= worker.activeTasks * 10.0;
    
    // Reward available memory
    if (worker.availableMemory > 0) {
        score += std::min(worker.availableMemory / 1024.0, 50.0); // Cap at 50
    }
    
    // Reward CPU cores
    score += worker.cpuCores * 2.0;
    
    // Reward GPU availability for GPU tasks
    if (task.requiresGpu && worker.hasGpu) {
        score += 30.0;
    }
    
    // Penalize if GPU required but not available
    if (task.requiresGpu && !worker.hasGpu) {
        score = 0.0;
    }
    
    // Reward Linux/Windows workers for complex tasks (better performance)
    if (task.complexity > 7.0) {
        if (worker.type == WorkerType::LINUX || worker.type == WorkerType::WINDOWS) {
            score += 20.0;
        }
    }
    
    return std::max(0.0, score);
}

std::string WorkerManager::generateWorkerId() {
    std::ostringstream oss;
    oss << "worker-" << nextWorkerId_++;
    return oss.str();
}

