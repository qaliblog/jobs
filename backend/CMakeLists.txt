cmake_minimum_required(VERSION 3.20)
project(JobsBackend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/server.cpp
    src/task_processor.cpp
    src/resource_monitor.cpp
    src/task_queue.cpp
    src/gpu_accelerator.cpp
    src/worker_manager.cpp
)

# Create executable
add_executable(jobs_backend ${SOURCES})

# Link libraries
target_link_libraries(jobs_backend
    PRIVATE
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(jobs_backend PRIVATE -Wall -Wextra -O3)
endif()

# GPU support (optional - requires CUDA or OpenCL)
option(ENABLE_CUDA "Enable CUDA support" OFF)
option(ENABLE_OPENCL "Enable OpenCL support" OFF)

if(ENABLE_CUDA)
    find_package(CUDA REQUIRED)
    enable_language(CUDA)
    target_link_libraries(jobs_backend PRIVATE ${CUDA_LIBRARIES})
    target_include_directories(jobs_backend PRIVATE ${CUDA_INCLUDE_DIRS})
endif()

if(ENABLE_OPENCL)
    find_package(OpenCL REQUIRED)
    target_link_libraries(jobs_backend PRIVATE OpenCL::OpenCL)
endif()

