#include "network_scanner.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <ifaddrs.h>
#include <net/if.h>
#endif

NetworkScanner::NetworkScanner() 
    : scanning_(false), broadcasting_(false), scan_port_(8080) {
}

NetworkScanner::~NetworkScanner() {
    stopScanning();
    stopBroadcasting();
}

void NetworkScanner::startScanning(int port) {
    if (scanning_) return;
    
    scan_port_ = port;
    scanning_ = true;
    scan_thread_ = std::make_unique<std::thread>(&NetworkScanner::scanLoop, this);
}

void NetworkScanner::stopScanning() {
    if (!scanning_) return;
    
    scanning_ = false;
    if (scan_thread_ && scan_thread_->joinable()) {
        scan_thread_->join();
    }
}

void NetworkScanner::startBroadcasting(const std::string& /* deviceName */,
                                      const std::string& /* deviceType */,
                                      int /* serverPort */, bool /* canServer */, bool /* canWorker */) {
    if (broadcasting_) return;
    
    broadcasting_ = true;
    broadcast_thread_ = std::make_unique<std::thread>(&NetworkScanner::broadcastLoop, this);
}

void NetworkScanner::stopBroadcasting() {
    if (!broadcasting_) return;
    
    broadcasting_ = false;
    if (broadcast_thread_ && broadcast_thread_->joinable()) {
        broadcast_thread_->join();
    }
}

void NetworkScanner::scanLoop() {
    while (scanning_) {
        scanNetwork(scan_port_);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void NetworkScanner::broadcastLoop() {
    // Broadcast device presence periodically
    while (broadcasting_) {
        // Implementation would send UDP broadcast packets
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

void NetworkScanner::scanNetwork(int port) {
    // Get local network prefix
    std::vector<std::string> networkPrefixes;
    
    #ifdef _WIN32
    // Windows implementation
    #else
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == 0) {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL) continue;
            if (ifa->ifa_addr->sa_family == AF_INET) {
                struct sockaddr_in* sa = (struct sockaddr_in*)ifa->ifa_addr;
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN);
                
                // Skip loopback
                if (strcmp(ip, "127.0.0.1") != 0) {
                    std::string ipStr(ip);
                    size_t lastDot = ipStr.find_last_of('.');
                    if (lastDot != std::string::npos) {
                        networkPrefixes.push_back(ipStr.substr(0, lastDot + 1));
                    }
                }
            }
        }
        freeifaddrs(ifaddr);
    }
    #endif
    
    // Scan common IP ranges
    if (networkPrefixes.empty()) {
        networkPrefixes.push_back("192.168.1.");
        networkPrefixes.push_back("192.168.0.");
        networkPrefixes.push_back("10.0.0.");
    }
    
    // Try to connect to each IP on the port
    for (const auto& prefix : networkPrefixes) {
        for (int i = 1; i < 255; ++i) {
            if (!scanning_) break;
            
            std::ostringstream ip;
            ip << prefix << i;
            
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) continue;
            
            struct sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            
            #ifdef _WIN32
            InetPtonA(AF_INET, ip.str().c_str(), &addr.sin_addr);
            #else
            inet_pton(AF_INET, ip.str().c_str(), &addr.sin_addr);
            #endif
            
            // Set non-blocking or timeout
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000; // 100ms
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
            setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
            
            if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                // Send discovery request
                std::string request = "GET /api/discover HTTP/1.1\r\n\r\n";
                send(sock, request.c_str(), request.length(), 0);
                
                char buffer[1024] = {0};
                ssize_t bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
                
                if (bytes > 0) {
                    std::string response(buffer, bytes);
                    handleDiscoveryResponse(response, ip.str());
                }
            }
            
            #ifdef _WIN32
            closesocket(sock);
            #else
            close(sock);
            #endif
        }
    }
}

void NetworkScanner::handleDiscoveryResponse(const std::string& response, const std::string& address) {
    // Parse discovery response
    // Format: {"id":"device-1","name":"My Device","type":"android","isServer":true,"isWorker":true}
    
    DiscoveredDevice device;
    device.address = address;
    
    // Simple JSON parsing
    size_t idPos = response.find("\"id\":\"");
    if (idPos != std::string::npos) {
        idPos += 6;
        size_t idEnd = response.find("\"", idPos);
        if (idEnd != std::string::npos) {
            device.id = response.substr(idPos, idEnd - idPos);
        }
    }
    
    size_t namePos = response.find("\"name\":\"");
    if (namePos != std::string::npos) {
        namePos += 8;
        size_t nameEnd = response.find("\"", namePos);
        if (nameEnd != std::string::npos) {
            device.name = response.substr(namePos, nameEnd - namePos);
        }
    }
    
    size_t typePos = response.find("\"type\":\"");
    if (typePos != std::string::npos) {
        typePos += 8;
        size_t typeEnd = response.find("\"", typePos);
        if (typeEnd != std::string::npos) {
            device.type = response.substr(typePos, typeEnd - typePos);
        }
    }
    
    device.isServer = (response.find("\"isServer\":true") != std::string::npos);
    device.isWorker = (response.find("\"isWorker\":true") != std::string::npos);
    
    // Check if already discovered
    std::lock_guard<std::mutex> lock(devices_mutex_);
    bool found = false;
    for (auto& d : discovered_devices_) {
        if (d.id == device.id || d.address == device.address) {
            d = device; // Update
            found = true;
            break;
        }
    }
    
    if (!found) {
        discovered_devices_.push_back(device);
        if (on_device_found_) {
            on_device_found_(device);
        }
    }
}

std::vector<DiscoveredDevice> NetworkScanner::getDiscoveredDevices() {
    std::lock_guard<std::mutex> lock(devices_mutex_);
    return discovered_devices_;
}

void NetworkScanner::setOnDeviceFound(std::function<void(const DiscoveredDevice&)> callback) {
    on_device_found_ = callback;
}

