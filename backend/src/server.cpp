#include "server.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <regex>
#include <thread>
#include <chrono>
#include <iterator>

// Simple JSON parsing (in production, use a proper JSON library)
std::string extractJsonValue(const std::string& json, const std::string& key) {
    std::regex pattern("\"" + key + "\"\\s*:\\s*\"?([^,\"\\}]+)\"?");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return match[1].str();
    }
    return "";
}

float extractJsonFloat(const std::string& json, const std::string& key) {
    std::regex pattern("\"" + key + "\"\\s*:\\s*([0-9]+\\.?[0-9]*)");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return std::stof(match[1].str());
    }
    return 0.0f;
}

int extractJsonInt(const std::string& json, const std::string& key) {
    std::regex pattern("\"" + key + "\"\\s*:\\s*([0-9]+)");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return std::stoi(match[1].str());
    }
    return 0;
}

bool extractJsonBool(const std::string& json, const std::string& key) {
    std::regex pattern("\"" + key + "\"\\s*:\\s*(true|false)");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return match[1].str() == "true";
    }
    return false;
}

Server::Server(int port) 
    : port_(port), running_(false) {
    task_queue_ = std::make_unique<TaskQueue>();
    task_processor_ = std::make_unique<TaskProcessor>(task_queue_.get(), 4);
    resource_monitor_ = std::make_unique<ResourceMonitor>();
    worker_manager_ = std::make_unique<WorkerManager>();
}

Server::~Server() {
    stop();
}

void Server::start() {
    if (running_) return;
    
    running_ = true;
    task_processor_->start();
    server_thread_ = std::make_unique<std::thread>(&Server::serverLoop, this);
    cleanup_thread_ = std::make_unique<std::thread>(&Server::cleanupLoop, this);
}

void Server::stop() {
    if (!running_) return;
    
    running_ = false;
    task_processor_->stop();
    
    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->join();
    }
    
    if (cleanup_thread_ && cleanup_thread_->joinable()) {
        cleanup_thread_->join();
    }
    
    // Wait for all client threads to finish
    for (auto& thread : client_threads_) {
        if (thread->joinable()) {
            thread->join();
        }
    }
    client_threads_.clear();
}

void Server::serverLoop() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind to port " << port_ << "\n";
        close(server_fd);
        return;
    }
    
    if (listen(server_fd, 10) < 0) {
        std::cerr << "Failed to listen\n";
        close(server_fd);
        return;
    }
    
    std::cout << "Server listening on port " << port_ << std::endl;
    
    while (running_) {
        sockaddr_in client_address{};
        socklen_t addr_len = sizeof(client_address);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_address, &addr_len);
        if (client_fd < 0) {
            if (running_) {
                std::cerr << "Failed to accept connection\n";
            }
            continue;
        }
        
        // Handle each client in a separate thread
        client_threads_.push_back(
            std::make_unique<std::thread>(&Server::handleClient, this, client_fd, client_address)
        );
        
        // Clean up finished threads
        client_threads_.erase(
            std::remove_if(client_threads_.begin(), client_threads_.end(),
                [](const std::unique_ptr<std::thread>& t) {
                    return !t->joinable();
                }),
            client_threads_.end()
        );
    }
    
    close(server_fd);
}

void Server::handleClient(int client_fd, sockaddr_in client_address) {
    char buffer[8192] = {0};
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    
    if (bytes_read > 0) {
        std::string request(buffer, bytes_read);
        std::string response;
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
        
        handleRequest(client_fd, request, response);
        
        send(client_fd, response.c_str(), response.length(), 0);
    }
    
    close(client_fd);
}

void Server::handleRequest(int client_fd, const std::string& request, std::string& response) {
    // Extract client address for worker registration
    sockaddr_in client_address{};
    socklen_t addr_len = sizeof(client_address);
    getpeername(client_fd, (struct sockaddr*)&client_address, &addr_len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
    std::string clientAddress(client_ip);
    
    // Simple HTTP parsing
    if (request.find("GET /health") != std::string::npos) {
        response = processHealthCheck();
    } else if (request.find("GET /api/resources") != std::string::npos) {
        response = processResourceQuery();
    } else if (request.find("GET /api/workers") != std::string::npos) {
        response = processWorkerList();
    } else if (request.find("POST /api/tasks") != std::string::npos) {
        size_t body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            std::string json = request.substr(body_start + 4);
            response = processTaskSubmission(json);
        } else {
            response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        }
    } else if (request.find("POST /api/workers/register") != std::string::npos) {
        size_t body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            std::string json = request.substr(body_start + 4);
            response = processWorkerRegister(json, clientAddress);
        } else {
            response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        }
    } else if (request.find("POST /api/workers/heartbeat") != std::string::npos) {
        size_t body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            std::string json = request.substr(body_start + 4);
            response = processWorkerHeartbeat(json);
        } else {
            response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        }
    } else if (request.find("GET /api/workers/task") != std::string::npos) {
        // Extract worker ID from query or header
        std::string workerId = extractJsonValue(request, "workerId");
        if (workerId.empty()) {
            // Try to extract from query string
            size_t query_start = request.find("workerId=");
            if (query_start != std::string::npos) {
                size_t query_end = request.find(" ", query_start);
                if (query_end != std::string::npos) {
                    workerId = request.substr(query_start + 9, query_end - query_start - 9);
                }
            }
        }
        response = processWorkerTaskRequest(workerId);
    } else if (request.find("POST /api/workers/complete") != std::string::npos) {
        size_t body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            std::string json = request.substr(body_start + 4);
            response = processWorkerTaskComplete(json);
        } else {
            response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        }
    } else if (request.find("GET /api/discover") != std::string::npos) {
        response = processDiscovery();
    } else if (request.find("POST /api/recruit") != std::string::npos) {
        size_t body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            std::string json = request.substr(body_start + 4);
            response = processRecruitRequest(json, clientAddress);
        } else {
            response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        }
    } else if (request.find("POST /api/work") != std::string::npos) {
        size_t body_start = request.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            std::string json = request.substr(body_start + 4);
            response = processWorkRequest(json, clientAddress);
        } else {
            response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        }
    } else {
        response = "HTTP/1.1 404 Not Found\r\n\r\n";
    }
}

std::string Server::processHealthCheck() {
    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "\r\n"
           "{\"status\":\"ok\"}";
}

std::string Server::processResourceQuery() {
    SystemResources resources = resource_monitor_->getCurrentResources();
    std::ostringstream json;
    json << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: application/json\r\n"
         << "Access-Control-Allow-Origin: *\r\n"
         << "\r\n"
         << "{\"cpuUsage\":" << resources.cpuUsage
         << ",\"memoryUsage\":" << resources.memoryUsage
         << ",\"gpuUsage\":" << resources.gpuUsage
         << ",\"availableMemory\":" << resources.availableMemory
         << ",\"totalMemory\":" << resources.totalMemory
         << ",\"cpuCores\":" << resources.cpuCores << "}";
    return json.str();
}

std::string Server::processTaskSubmission(const std::string& json) {
    Task task;
    task.id = extractJsonValue(json, "id");
    task.type = extractJsonValue(json, "type");
    task.complexity = extractJsonFloat(json, "complexity");
    task.dataSize = extractJsonInt(json, "dataSize");
    task.requiresGpu = extractJsonBool(json, "requiresGpu");
    task.data = json;
    
    if (task.id.empty()) {
        return "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\":\"Invalid task\"}";
    }
    
    // Try to find a worker for this task
    std::string workerId = worker_manager_->selectBestWorker(task);
    
    if (!workerId.empty()) {
        // Assign to worker
        worker_manager_->incrementWorkerTasks(workerId);
        task_queue_->push(task);
        
        std::ostringstream response;
        response << "HTTP/1.1 202 Accepted\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Access-Control-Allow-Origin: *\r\n"
                 << "\r\n"
                 << "{\"taskId\":\"" << task.id 
                 << "\",\"status\":\"assigned\",\"workerId\":\"" << workerId << "\"}";
        return response.str();
    } else {
        // No workers available, use local processing
        task_queue_->push(task);
        
        std::ostringstream response;
        response << "HTTP/1.1 202 Accepted\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Access-Control-Allow-Origin: *\r\n"
                 << "\r\n"
                 << "{\"taskId\":\"" << task.id << "\",\"status\":\"queued\",\"workerId\":\"local\"}";
        return response.str();
    }
}

std::string Server::processWorkerRegister(const std::string& json, const std::string& clientAddress) {
    std::string workerType = extractJsonValue(json, "type");
    int port = extractJsonInt(json, "port");
    std::string capabilities = extractJsonValue(json, "capabilities");
    
    if (workerType.empty()) {
        return "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\":\"Missing worker type\"}";
    }
    
    std::string workerId = worker_manager_->registerWorker(workerType, clientAddress, port, capabilities);
    
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: application/json\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "\r\n"
             << "{\"workerId\":\"" << workerId << "\",\"status\":\"registered\"}";
    return response.str();
}

std::string Server::processWorkerHeartbeat(const std::string& json) {
    std::string workerId = extractJsonValue(json, "workerId");
    double cpuUsage = extractJsonFloat(json, "cpuUsage");
    long long availableMemory = extractJsonInt(json, "availableMemory");
    
    if (workerId.empty()) {
        return "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\":\"Missing worker ID\"}";
    }
    
    bool updated = worker_manager_->updateWorkerHeartbeat(workerId);
    if (updated) {
        worker_manager_->updateWorkerResources(workerId, cpuUsage, availableMemory);
    }
    
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: application/json\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "\r\n"
             << "{\"status\":\"" << (updated ? "ok" : "not_found") << "\"}";
    return response.str();
}

std::string Server::processWorkerList() {
    std::vector<WorkerInfo> workers = worker_manager_->getAllWorkers();
    
    std::ostringstream json;
    json << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: application/json\r\n"
         << "Access-Control-Allow-Origin: *\r\n"
         << "\r\n"
         << "{\"workers\":[";
    
    bool first = true;
    for (const auto& worker : workers) {
        if (!first) json << ",";
        first = false;
        
        std::string typeStr = "unknown";
        if (worker.type == WorkerType::ANDROID) typeStr = "android";
        else if (worker.type == WorkerType::LINUX) typeStr = "linux";
        else if (worker.type == WorkerType::WINDOWS) typeStr = "windows";
        
        json << "{\"id\":\"" << worker.id
             << "\",\"type\":\"" << typeStr
             << "\",\"address\":\"" << worker.address
             << "\",\"port\":" << worker.port
             << ",\"isActive\":" << (worker.isActive ? "true" : "false")
             << ",\"cpuCores\":" << worker.cpuCores
             << ",\"cpuUsage\":" << worker.cpuUsage
             << ",\"availableMemory\":" << worker.availableMemory
             << ",\"hasGpu\":" << (worker.hasGpu ? "true" : "false")
             << ",\"activeTasks\":" << worker.activeTasks
             << ",\"completedTasks\":" << worker.completedTasks << "}";
    }
    
    json << "],\"total\":" << workers.size() << "}";
    return json.str();
}

std::string Server::processWorkerTaskRequest(const std::string& workerId) {
    if (workerId.empty()) {
        return "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\":\"Missing worker ID\"}";
    }
    
    // Get a task from the queue for this worker
    Task task;
    if (task_queue_->pop(task, 1000)) {
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: application/json\r\n"
                 << "Access-Control-Allow-Origin: *\r\n"
                 << "\r\n"
                 << "{\"taskId\":\"" << task.id
                 << "\",\"type\":\"" << task.type
                 << "\",\"complexity\":" << task.complexity
                 << ",\"dataSize\":" << task.dataSize
                 << ",\"requiresGpu\":" << (task.requiresGpu ? "true" : "false")
                 << ",\"data\":" << task.data << "}";
        return response.str();
    } else {
        return "HTTP/1.1 204 No Content\r\n\r\n";
    }
}

std::string Server::processWorkerTaskComplete(const std::string& json) {
    std::string workerId = extractJsonValue(json, "workerId");
    std::string taskId = extractJsonValue(json, "taskId");
    std::string result = extractJsonValue(json, "result");
    
    if (workerId.empty() || taskId.empty()) {
        return "HTTP/1.1 400 Bad Request\r\n\r\n{\"error\":\"Missing required fields\"}";
    }
    
    worker_manager_->decrementWorkerTasks(workerId);
    worker_manager_->incrementWorkerCompleted(workerId);
    
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: application/json\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "\r\n"
             << "{\"status\":\"completed\",\"taskId\":\"" << taskId << "\"}";
    return response.str();
}

void Server::cleanupLoop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        worker_manager_->cleanupInactiveWorkers(30);
    }
}

std::string Server::processDiscovery() {
    std::ostringstream json;
    json << "HTTP/1.1 200 OK\r\n"
         << "Content-Type: application/json\r\n"
         << "Access-Control-Allow-Origin: *\r\n"
         << "\r\n"
         << "{\"id\":\"server-" << port_
         << "\",\"name\":\"Jobs Server\",\"type\":\"server\""
         << ",\"isServer\":true,\"isWorker\":true"
         << ",\"port\":" << port_ << "}";
    return json.str();
}

std::string Server::processRecruitRequest(const std::string& json, const std::string& clientAddress) {
    // Someone wants to recruit this server as a worker
    std::string requesterId = extractJsonValue(json, "requesterId");
    std::string requesterName = extractJsonValue(json, "requesterName");
    std::string requesterAddress = extractJsonValue(json, "requesterAddress");
    int requesterPort = extractJsonInt(json, "requesterPort");
    
    // In a real implementation, this would show a prompt to the user
    // For now, auto-accept and register as worker
    std::string capabilities = "cpuCores:" + std::to_string(resource_monitor_->getCurrentResources().cpuCores) +
                             ",availableMemory:" + std::to_string(resource_monitor_->getCurrentResources().availableMemory) +
                             ",hasGpu:false";
    
    // Register this server as a worker for the requester
    // The requester's address is in the request
    
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: application/json\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "\r\n"
             << "{\"status\":\"accepted\",\"message\":\"Ready to work\"}";
    return response.str();
}

std::string Server::processWorkRequest(const std::string& json, const std::string& clientAddress) {
    // Someone wants to work for this server
    std::string workerId = extractJsonValue(json, "workerId");
    std::string workerType = extractJsonValue(json, "workerType");
    std::string capabilities = extractJsonValue(json, "capabilities");
    
    // Auto-accept and register the worker
    std::string registeredId = worker_manager_->registerWorker(workerType, clientAddress, 0, capabilities);
    
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: application/json\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "\r\n"
             << "{\"status\":\"accepted\",\"workerId\":\"" << registeredId << "\"}";
    return response.str();
}

