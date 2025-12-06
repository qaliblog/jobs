#include <iostream>
#include <csignal>
#include <string>
#include <thread>
#include <chrono>
#include "worker_client.h"
#include "request_handler.h"
#include "network_scanner.h"

std::unique_ptr<WorkerClient> g_worker;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down...\n";
    if (g_worker) {
        g_worker->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    std::string serverAddress = "localhost";
    int serverPort = 8080;
    std::string workerType = "linux";
    
    #ifdef _WIN32
    workerType = "windows";
    #endif
    
    if (argc > 1) {
        serverAddress = argv[1];
    }
    if (argc > 2) {
        serverPort = std::stoi(argv[2]);
    }
    if (argc > 3) {
        workerType = argv[3];
    }
    
    std::cout << "Starting Jobs Worker Client" << std::endl;
    std::cout << "Server: " << serverAddress << ":" << serverPort << std::endl;
    std::cout << "Type: " << workerType << std::endl;
    
    // Register signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Network scanner for discovery
    NetworkScanner scanner;
    RequestHandler requestHandler;
    
    std::cout << "\nOptions:" << std::endl;
    std::cout << "1. Connect to specific server" << std::endl;
    std::cout << "2. Scan network for devices" << std::endl;
    std::cout << "3. Start as server (accept requests)" << std::endl;
    std::cout << "Choice (1-3): ";
    
    std::string choice;
    std::getline(std::cin, choice);
    
    if (choice == "2") {
        // Scan network
        std::cout << "Scanning network..." << std::endl;
        scanner.startScanning(8080);
        std::this_thread::sleep_for(std::chrono::seconds(5));
        scanner.stopScanning();
        
        auto devices = scanner.getDiscoveredDevices();
        std::cout << "Found " << devices.size() << " devices:" << std::endl;
        for (size_t i = 0; i < devices.size(); ++i) {
            const auto& device = devices[i];
            std::cout << (i + 1) << ". " << device.name 
                      << " (" << device.type << ") @ " 
                      << device.address << ":" << device.port << std::endl;
        }
        
        if (!devices.empty()) {
            std::cout << "Select device (1-" << devices.size() << "): ";
            std::string selection;
            std::getline(std::cin, selection);
            int idx = std::stoi(selection) - 1;
            if (idx >= 0 && idx < devices.size()) {
                serverAddress = devices[idx].address;
                serverPort = devices[idx].port;
            }
        }
    } else if (choice == "3") {
        // Start as server (would need server implementation)
        std::cout << "Server mode not yet implemented in worker client" << std::endl;
        return 0;
    }
    
    g_worker = std::make_unique<WorkerClient>(serverAddress, serverPort, workerType);
    
    if (!g_worker->connect()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    g_worker->start();
    
    std::cout << "Worker started. Press Ctrl+C to stop.\n";
    
    // Keep main thread alive
    while (g_worker->isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}

