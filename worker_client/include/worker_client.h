#ifndef WORKER_CLIENT_H
#define WORKER_CLIENT_H

#include <string>
#include <thread>
#include <atomic>
#include <memory>

class WorkerClient {
public:
    WorkerClient(const std::string& serverAddress, int serverPort,
                const std::string& workerType);
    ~WorkerClient();
    
    bool connect();
    void disconnect();
    void start();
    void stop();
    bool isRunning() const { return running_; }
    
    std::string getWorkerId() const { return worker_id_; }
    
private:
    std::string server_address_;
    int server_port_;
    std::string worker_type_;
    std::string worker_id_;
    std::atomic<bool> running_;
    std::atomic<bool> connected_;
    
    std::unique_ptr<std::thread> heartbeat_thread_;
    std::unique_ptr<std::thread> task_thread_;
    
    bool registerWorker();
    void heartbeatLoop();
    void taskLoop();
    bool sendHttpRequest(const std::string& method, const std::string& path,
                       const std::string& body, std::string& response);
    std::string processTask(const std::string& taskJson);
    std::string extractTaskId(const std::string& json);
};

#endif // WORKER_CLIENT_H

