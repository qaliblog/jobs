#include "worker_client.h"
#include "resource_monitor.h"
#include "task_executor.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#define close closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

WorkerClient::WorkerClient(const std::string& serverAddress, int serverPort,
                          const std::string& workerType)
    : server_address_(serverAddress), server_port_(serverPort),
      worker_type_(workerType), running_(false), connected_(false) {
    
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif
}

WorkerClient::~WorkerClient() {
    stop();
    #ifdef _WIN32
    WSACleanup();
    #endif
}

bool WorkerClient::connect() {
    return registerWorker();
}

void WorkerClient::disconnect() {
    connected_ = false;
}

void WorkerClient::start() {
    if (running_) return;
    
    running_ = true;
    connected_ = true;
    
    heartbeat_thread_ = std::make_unique<std::thread>(&WorkerClient::heartbeatLoop, this);
    task_thread_ = std::make_unique<std::thread>(&WorkerClient::taskLoop, this);
}

void WorkerClient::stop() {
    if (!running_) return;
    
    running_ = false;
    connected_ = false;
    
    if (heartbeat_thread_ && heartbeat_thread_->joinable()) {
        heartbeat_thread_->join();
    }
    
    if (task_thread_ && task_thread_->joinable()) {
        task_thread_->join();
    }
}

bool WorkerClient::registerWorker() {
    ResourceMonitor monitor;
    WorkerResources resources = monitor.getCurrentResources();
    
    std::ostringstream capabilities;
    capabilities << "cpuCores:" << resources.cpuCores
                 << ",availableMemory:" << resources.availableMemory
                 << ",hasGpu:false"; // Could detect GPU here
    
    std::ostringstream json;
    json << "{\"type\":\"" << worker_type_
         << "\",\"port\":0"
         << ",\"capabilities\":\"" << capabilities.str() << "\"}";
    
    std::string response;
    if (!sendHttpRequest("POST", "/api/workers/register", json.str(), response)) {
        return false;
    }
    
    // Extract worker ID from response
    size_t idPos = response.find("\"workerId\":\"");
    if (idPos != std::string::npos) {
        idPos += 11;
        size_t idEnd = response.find("\"", idPos);
        if (idEnd != std::string::npos) {
            worker_id_ = response.substr(idPos, idEnd - idPos);
            std::cout << "Registered as worker: " << worker_id_ << std::endl;
            return true;
        }
    }
    
    return false;
}

void WorkerClient::heartbeatLoop() {
    ResourceMonitor monitor;
    
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        if (!connected_) continue;
        
        WorkerResources resources = monitor.getCurrentResources();
        
        std::ostringstream json;
        json << "{\"workerId\":\"" << worker_id_
             << "\",\"cpuUsage\":" << resources.cpuUsage
             << ",\"availableMemory\":" << resources.availableMemory << "}";
        
        std::string response;
        sendHttpRequest("POST", "/api/workers/heartbeat", json.str(), response);
    }
}

void WorkerClient::taskLoop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        if (!connected_) continue;
        
        std::ostringstream request;
        request << "GET /api/workers/task?workerId=" << worker_id_ << " HTTP/1.1\r\n"
                << "Host: " << server_address_ << ":" << server_port_ << "\r\n"
                << "Connection: keep-alive\r\n\r\n";
        
        std::string response;
        if (sendHttpRequest("GET", "/api/workers/task?workerId=" + worker_id_, "", response)) {
            if (response.find("HTTP/1.1 200") != std::string::npos) {
                // Extract task JSON from response
                size_t bodyStart = response.find("\r\n\r\n");
                if (bodyStart != std::string::npos) {
                    std::string taskJson = response.substr(bodyStart + 4);
                    std::string result = processTask(taskJson);
                    
                    // Send completion
                    std::ostringstream completeJson;
                    completeJson << "{\"workerId\":\"" << worker_id_
                                 << "\",\"taskId\":\""
                                 << extractTaskId(taskJson)
                                 << "\",\"result\":\"" << result << "\"}";
                    
                    std::string completeResponse;
                    sendHttpRequest("POST", "/api/workers/complete", completeJson.str(), completeResponse);
                }
            }
        }
    }
}

std::string WorkerClient::processTask(const std::string& taskJson) {
    return TaskExecutor::executeTask(taskJson);
}

std::string WorkerClient::extractTaskId(const std::string& json) {
    size_t pos = json.find("\"taskId\":\"");
    if (pos != std::string::npos) {
        pos += 10;
        size_t end = json.find("\"", pos);
        if (end != std::string::npos) {
            return json.substr(pos, end - pos);
        }
    }
    return "";
}

bool WorkerClient::sendHttpRequest(const std::string& method, const std::string& path,
                                  const std::string& body, std::string& response) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return false;
    }
    
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port_);
    
    #ifdef _WIN32
    InetPtonA(AF_INET, server_address_.c_str(), &server_addr.sin_addr);
    #else
    inet_pton(AF_INET, server_address_.c_str(), &server_addr.sin_addr);
    #endif
    
    if (::connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        #ifdef _WIN32
        closesocket(sock);
        #else
        close(sock);
        #endif
        return false;
    }
    
    std::ostringstream request;
    request << method << " " << path << " HTTP/1.1\r\n"
            << "Host: " << server_address_ << ":" << server_port_ << "\r\n"
            << "Content-Type: application/json\r\n"
            << "Content-Length: " << body.length() << "\r\n"
            << "Connection: close\r\n\r\n"
            << body;
    
    std::string requestStr = request.str();
    send(sock, requestStr.c_str(), requestStr.length(), 0);
    
    char buffer[8192] = {0};
#ifdef _WIN32
    int bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0);
#else
    ssize_t bytesRead = recv(sock, buffer, sizeof(buffer) - 1, 0);
#endif
    
    if (bytesRead > 0) {
        response = std::string(buffer, bytesRead);
    }
    
    #ifdef _WIN32
    closesocket(sock);
    #else
    close(sock);
    #endif
    
    return bytesRead > 0;
}

