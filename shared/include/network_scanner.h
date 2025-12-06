#ifndef NETWORK_SCANNER_H
#define NETWORK_SCANNER_H

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>

struct DiscoveredDevice {
    std::string id;
    std::string name;
    std::string address;
    int port;
    std::string type; // "android", "linux", "windows"
    bool isServer;   // Can act as coordinator
    bool isWorker;    // Can act as worker
    std::string capabilities;
    
    DiscoveredDevice() : port(0), isServer(false), isWorker(false) {}
};

class NetworkScanner {
public:
    NetworkScanner();
    ~NetworkScanner();
    
    void startScanning(int port = 8080);
    void stopScanning();
    std::vector<DiscoveredDevice> getDiscoveredDevices();
    void setOnDeviceFound(std::function<void(const DiscoveredDevice&)> callback);
    
    // Broadcast this device's presence
    void startBroadcasting(const std::string& deviceName, const std::string& deviceType,
                          int serverPort, bool canServer, bool canWorker);
    void stopBroadcasting();
    
private:
    std::atomic<bool> scanning_;
    std::atomic<bool> broadcasting_;
    std::unique_ptr<std::thread> scan_thread_;
    std::unique_ptr<std::thread> broadcast_thread_;
    std::vector<DiscoveredDevice> discovered_devices_;
    std::mutex devices_mutex_;
    std::function<void(const DiscoveredDevice&)> on_device_found_;
    int scan_port_;
    
    void scanLoop();
    void broadcastLoop();
    void scanNetwork(int port);
    void handleDiscoveryResponse(const std::string& response, const std::string& address);
};

#endif // NETWORK_SCANNER_H

