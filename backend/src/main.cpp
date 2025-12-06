#include <iostream>
#include <csignal>
#include <memory>
#include "server.h"

std::unique_ptr<Server> g_server;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down...\n";
    if (g_server) {
        g_server->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    
    std::cout << "Starting Jobs Backend Server on port " << port << std::endl;
    
    // Register signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    g_server = std::make_unique<Server>(port);
    g_server->start();
    
    std::cout << "Server started. Press Ctrl+C to stop.\n";
    
    // Keep main thread alive
    while (g_server->isRunning()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}

