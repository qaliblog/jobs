#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include "task_queue.h"
#include "task_processor.h"
#include "resource_monitor.h"
#include "worker_manager.h"
#include <map>
#include <mutex>

class Server {
public:
    Server(int port);
    ~Server();
    
    void start();
    void stop();
    bool isRunning() const { return running_; }
    
private:
    int port_;
    std::atomic<bool> running_;
    std::unique_ptr<std::thread> server_thread_;
    std::unique_ptr<std::thread> cleanup_thread_;
    std::unique_ptr<TaskQueue> task_queue_;
    std::unique_ptr<TaskProcessor> task_processor_;
    std::unique_ptr<ResourceMonitor> resource_monitor_;
    std::unique_ptr<WorkerManager> worker_manager_;
    std::vector<std::unique_ptr<std::thread>> client_threads_;
    
    // Pending connection requests (both recruit and work)
    struct ConnectionRequest {
        std::string id;
        std::string deviceName;
        std::string deviceAddress;
        std::string deviceType;
        std::string requestType; // "recruit" or "work"
        std::chrono::system_clock::time_point timestamp;

        // For JSON serialization
        std::string toJson() const {
            std::ostringstream ss;
            auto time_t = std::chrono::system_clock::to_time_t(timestamp);
            ss << "{\"id\":\"" << id << "\","
               << "\"name\":\"" << deviceName << "\","
               << "\"address\":\"" << deviceAddress << "\","
               << "\"type\":\"" << deviceType << "\","
               << "\"requestType\":\"" << requestType << "\","
               << "\"timestamp\":" << time_t << "}";
            return ss.str();
        }
    };
    std::vector<ConnectionRequest> pending_requests_;
    std::mutex requests_mutex_;
    
    void serverLoop();
    void handleClient(int client_fd, sockaddr_in client_address);
    void handleRequest(int client_fd, const std::string& request, std::string& response);
    void cleanupLoop();
    
    std::string processHealthCheck();
    std::string processResourceQuery();
    std::string processTaskSubmission(const std::string& json);
    std::string processWorkerRegister(const std::string& json, const std::string& clientAddress);
    std::string processWorkerHeartbeat(const std::string& json);
    std::string processWorkerList();
    std::string processWorkerTaskRequest(const std::string& workerId);
    std::string processWorkerTaskComplete(const std::string& json);
    std::string processDiscovery();
    std::string processRecruitRequest(const std::string& json, const std::string& clientAddress);
    std::string processWorkRequest(const std::string& json, const std::string& clientAddress);
    std::string processPendingRequests();
    std::string processAcceptRequest(const std::string& json);
    std::string processRejectRequest(const std::string& json);
};

#endif // SERVER_H

