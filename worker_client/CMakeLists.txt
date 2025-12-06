cmake_minimum_required(VERSION 3.20)
project(JobsWorker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/include)

# Source files
set(SOURCES
    src/main.cpp
    src/worker_client.cpp
    src/resource_monitor.cpp
    src/task_executor.cpp
    src/request_handler.cpp
    ../shared/src/network_scanner.cpp
)

# Create executable
add_executable(jobs_worker ${SOURCES})

# Link libraries
find_package(Threads REQUIRED)
target_link_libraries(jobs_worker PRIVATE Threads::Threads)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(jobs_worker PRIVATE -Wall -Wextra -O3)
endif()

# Platform-specific
if(WIN32)
    target_link_libraries(jobs_worker PRIVATE ws2_32)
else()
    # Linux/Unix
endif()

