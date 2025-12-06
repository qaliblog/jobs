#include "task_processor.h"
#include "gpu_accelerator.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <thread>
#include <sstream>

TaskProcessor::TaskProcessor(TaskQueue* queue, int numThreads)
    : queue_(queue), num_threads_(numThreads), running_(false),
      processed_count_(0), total_processing_time_ms_(0) {
}

TaskProcessor::~TaskProcessor() {
    stop();
}

void TaskProcessor::start() {
    if (running_) return;
    
    running_ = true;
    for (int i = 0; i < num_threads_; ++i) {
        worker_threads_.push_back(
            std::make_unique<std::thread>(&TaskProcessor::workerLoop, this)
        );
    }
    
    std::cout << "Task processor started with " << num_threads_ << " threads\n";
}

void TaskProcessor::stop() {
    if (!running_) return;
    
    running_ = false;
    
    for (auto& thread : worker_threads_) {
        if (thread->joinable()) {
            thread->join();
        }
    }
    
    worker_threads_.clear();
}

void TaskProcessor::workerLoop() {
    Task task;
    GpuAccelerator gpu_accelerator;
    
    while (running_) {
        if (queue_->pop(task, 1000)) {
            auto start_time = std::chrono::high_resolution_clock::now();
            
            std::string result = processTask(task);
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - start_time).count();
            
            processed_count_++;
            total_processing_time_ms_ += duration;
            
            std::cout << "Processed task " << task.id 
                      << " in " << duration << "ms\n";
        }
    }
}

std::string TaskProcessor::processTask(const Task& task) {
    if (task.type == "computation") {
        if (task.requiresGpu) {
            return processGpuTask(task);
        } else {
            return processComputationTask(task);
        }
    }
    
    return "{\"result\":\"unknown_task_type\"}";
}

std::string TaskProcessor::processComputationTask(const Task& task) {
    // Simulate intensive computation
    // In a real scenario, this would perform actual computations
    int iterations = static_cast<int>(task.complexity * 1000000);
    
    // CPU-intensive computation
    double result = 0.0;
    for (int i = 0; i < iterations; ++i) {
        result += std::sin(i) * std::cos(i);
    }
    
    // Use multithreading for parallel computation
    const int num_cores = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::vector<double> partial_results(num_cores, 0.0);
    
    int work_per_thread = iterations / num_cores;
    for (int t = 0; t < num_cores; ++t) {
        threads.emplace_back([&, t, work_per_thread]() {
            int start = t * work_per_thread;
            int end = (t == num_cores - 1) ? iterations : (t + 1) * work_per_thread;
            for (int i = start; i < end; ++i) {
                partial_results[t] += std::sin(i) * std::cos(i);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    result = 0.0;
    for (double partial : partial_results) {
        result += partial;
    }
    
    std::ostringstream json;
    json << "{\"taskId\":\"" << task.id 
         << "\",\"result\":" << result
         << ",\"processingTime\":" << (task.complexity * 100) << "}";
    
    return json.str();
}

std::string TaskProcessor::processGpuTask(const Task& task) {
    GpuAccelerator gpu;
    
    if (gpu.isAvailable()) {
        std::string result = gpu.processTask(task.data, task.complexity);
        std::ostringstream json;
        json << "{\"taskId\":\"" << task.id 
             << "\",\"result\":\"" << result
             << "\",\"processingTime\":" << (task.complexity * 50) << "}";
        return json.str();
    } else {
        // Fallback to CPU
        return processComputationTask(task);
    }
}

double TaskProcessor::getAverageProcessingTime() const {
    size_t count = processed_count_.load();
    if (count == 0) return 0.0;
    return static_cast<double>(total_processing_time_ms_.load()) / count;
}

