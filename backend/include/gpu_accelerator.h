#ifndef GPU_ACCELERATOR_H
#define GPU_ACCELERATOR_H

#include <string>
#include <vector>

class GpuAccelerator {
public:
    GpuAccelerator();
    ~GpuAccelerator();
    
    bool isAvailable() const { return available_; }
    std::string processTask(const std::string& taskData, float complexity);
    
private:
    bool available_;
    void initializeGpu();
    void cleanupGpu();
    
    // GPU computation methods
    std::vector<float> performGpuComputation(const std::vector<float>& data, int iterations);
};

#endif // GPU_ACCELERATOR_H

