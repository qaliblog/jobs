#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include "task_queue.h"
#include "task_processor.h"
#include "resource_monitor.h"
#include "worker_manager.h"

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
};

#endif // SERVER_H

