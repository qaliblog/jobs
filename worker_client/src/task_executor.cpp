#include "task_executor.h"
#include <sstream>
#include <cmath>
#include <thread>
#include <algorithm>
#include <regex>

// Simple JSON parsing
float extractFloat(const std::string& json, const std::string& key) {
    std::regex pattern("\"" + key + "\"\\s*:\\s*([0-9]+\\.?[0-9]*)");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return std::stof(match[1].str());
    }
    return 0.0f;
}

int extractInt(const std::string& json, const std::string& key) {
    std::regex pattern("\"" + key + "\"\\s*:\\s*([0-9]+)");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return std::stoi(match[1].str());
    }
    return 0;
}

std::string TaskExecutor::executeTask(const std::string& taskJson) {
    std::string type = "";
    size_t typePos = taskJson.find("\"type\":\"");
    if (typePos != std::string::npos) {
        typePos += 8;
        size_t typeEnd = taskJson.find("\"", typePos);
        if (typeEnd != std::string::npos) {
            type = taskJson.substr(typePos, typeEnd - typePos);
        }
    }
    
    float complexity = extractFloat(taskJson, "complexity");
    int dataSize = extractInt(taskJson, "dataSize");
    bool requiresGpu = (taskJson.find("\"requiresGpu\":true") != std::string::npos);
    
    if (type == "computation") {
        if (requiresGpu) {
            return executeGpuTask(complexity, taskJson);
        } else {
            return executeComputationTask(complexity, dataSize);
        }
    }
    
    return "unknown_task_type";
}

std::string TaskExecutor::executeComputationTask(float complexity, int dataSize) {
    // Simulate intensive computation
    int iterations = static_cast<int>(complexity * 1000000);
    
    // CPU-intensive computation with multithreading
    const int numCores = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::vector<double> partialResults(numCores, 0.0);
    
    int workPerThread = iterations / numCores;
    for (int t = 0; t < numCores; ++t) {
        threads.emplace_back([&, t, workPerThread, iterations]() {
            int start = t * workPerThread;
            int end = (t == numCores - 1) ? iterations : (t + 1) * workPerThread;
            for (int i = start; i < end; ++i) {
                partialResults[t] += std::sin(i) * std::cos(i);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    double result = 0.0;
    for (double partial : partialResults) {
        result += partial;
    }
    
    std::ostringstream oss;
    oss << "result:" << result;
    return oss.str();
}

std::string TaskExecutor::executeGpuTask(float complexity, const std::string& data) {
    // GPU task execution (simulated)
    // In production, this would use CUDA/OpenCL
    int iterations = static_cast<int>(complexity * 500000);
    
    double result = 0.0;
    for (int i = 0; i < iterations; ++i) {
        result += std::sin(i) * std::cos(i);
    }
    
    std::ostringstream oss;
    oss << "gpu_result:" << result;
    return oss.str();
}

