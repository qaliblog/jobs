#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <memory>

struct Task {
    std::string id;
    std::string type;
    float complexity;
    int dataSize;
    bool requiresGpu;
    std::string data; // Task payload
    
    Task() : complexity(0.0f), dataSize(0), requiresGpu(false) {}
};

class TaskQueue {
public:
    void push(const Task& task);
    bool pop(Task& task, int timeoutMs = 1000);
    size_t size() const;
    bool empty() const;
    void clear();
    
private:
    std::queue<Task> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
};

#endif // TASK_QUEUE_H

