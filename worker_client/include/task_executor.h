#ifndef TASK_EXECUTOR_H
#define TASK_EXECUTOR_H

#include <string>

class TaskExecutor {
public:
    static std::string executeTask(const std::string& taskJson);
    
private:
    static std::string executeComputationTask(float complexity, int dataSize);
    static std::string executeGpuTask(float complexity, const std::string& data);
};

#endif // TASK_EXECUTOR_H

