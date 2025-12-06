cmake_minimum_required(VERSION 3.20)
project(JobsWorkerGUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 or Qt5
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets Network)
if(QT_FOUND)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network)
else()
    message(FATAL_ERROR "Qt not found. Please install Qt 6.x or Qt 5.x")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/include)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/network_scanner_widget.cpp
    src/connection_widget.cpp
    src/resource_monitor_widget.cpp
    src/task_status_widget.cpp
    src/worker_client_wrapper.cpp
    ../shared/src/network_scanner.cpp
)

set(HEADERS
    include/mainwindow.h
    include/network_scanner_widget.h
    include/connection_widget.h
    include/resource_monitor_widget.h
    include/task_status_widget.h
    include/worker_client_wrapper.h
)

set(RESOURCE_FILES
    resources/resources.qrc
)

# Create executable
add_executable(jobs_worker_gui ${SOURCES} ${HEADERS} ${RESOURCE_FILES})

# Link Qt libraries
target_link_libraries(jobs_worker_gui
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(jobs_worker_gui PRIVATE -Wall -Wextra -O3)
endif()

# Platform-specific
if(WIN32)
    set_target_properties(jobs_worker_gui PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    target_link_libraries(jobs_worker_gui PRIVATE ws2_32)
endif()

