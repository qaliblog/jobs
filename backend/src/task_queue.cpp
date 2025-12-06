#include "task_queue.h"
#include <chrono>

void TaskQueue::push(const Task& task) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(task);
    condition_.notify_one();
}

bool TaskQueue::pop(Task& task, int timeoutMs) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    if (condition_.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                           [this] { return !queue_.empty(); })) {
        task = queue_.front();
        queue_.pop();
        return true;
    }
    
    return false;
}

size_t TaskQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

bool TaskQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

void TaskQueue::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!queue_.empty()) {
        queue_.pop();
    }
}

