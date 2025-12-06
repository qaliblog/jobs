#ifndef TASK_PROCESSOR_H
#define TASK_PROCESSOR_H

#include "task_queue.h"
#include <thread>
#include <atomic>
#include <vector>
#include <memory>

class TaskProcessor {
public:
    TaskProcessor(TaskQueue* queue, int numThreads = 4);
    ~TaskProcessor();
    
    void start();
    void stop();
    bool isRunning() const { return running_; }
    
    // Statistics
    size_t getProcessedCount() const { return processed_count_; }
    double getAverageProcessingTime() const;
    
private:
    TaskQueue* queue_;
    int num_threads_;
    std::atomic<bool> running_;
    std::vector<std::unique_ptr<std::thread>> worker_threads_;
    std::atomic<size_t> processed_count_;
    std::atomic<long long> total_processing_time_ms_;
    
    void workerLoop();
    std::string processTask(const Task& task);
    std::string processComputationTask(const Task& task);
    std::string processGpuTask(const Task& task);
};

#endif // TASK_PROCESSOR_H

