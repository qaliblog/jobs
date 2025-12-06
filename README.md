# Jobs - Distributed Computing Platform

A high-performance distributed computing system that enables seamless collaboration between Android mobile devices and C++ desktop/laptop backends for compute-intensive tasks.

## Architecture

- **Android Client (Kotlin)**: UI, resource monitoring, and intelligent task delegation
- **C++ Backend**: High-performance task processing with GPU acceleration and multithreading
- **Smart Offloading**: Dynamic decision-making based on device resources, network conditions, and task complexity

## Features

- Real-time resource monitoring (CPU, GPU, memory, network)
- Intelligent task offloading algorithm
- GPU acceleration support
- Multithreaded processing
- **Multi-worker support**: Android, Linux, and Windows workers
- **Distributed task processing**: Automatic load balancing across workers
- **Worker health monitoring**: Automatic cleanup of inactive workers
- **Peer-to-peer architecture**: Any device can be coordinator or worker
- **Network discovery**: Automatic scanning to find other devices
- **Bidirectional requests**: Recruit others or work for others
- **Modern GUIs**: Beautiful interfaces for Android, Linux, and Windows
- Secure communication layer
- Efficient data transfer optimization

## Project Structure

```
jobs/
├── android/          # Android Kotlin application
│   ├── app/         # Main application module
│   └── build.gradle.kts
├── backend/          # C++ desktop/laptop backend
│   ├── include/     # Header files
│   ├── src/         # Source files
│   └── CMakeLists.txt
├── worker_client/      # Linux/Windows worker client (CLI)
├── worker_client_gui/  # Linux/Windows worker client (GUI)
├── shared/             # Shared network scanning utilities
├── README.md
├── QUICKSTART.md    # Quick start guide
├── BUILD.md         # Build instructions
├── ARCHITECTURE.md  # System architecture
├── SMART_OFFLOADING.md # Offloading algorithm details
└── MULTI_WORKER.md  # Multi-worker system guide
```

## Building

### Android App
```bash
cd android
./gradlew build
```

### C++ Backend
```bash
cd backend
mkdir build && cd build
cmake ..
make
```

## Quick Start

See [QUICKSTART.md](QUICKSTART.md) for detailed setup instructions.

1. Start the C++ backend server on your laptop
2. Launch the Android app
3. Connect to the backend via WiFi
4. The app will automatically offload tasks based on resource availability

## Documentation

- [QUICKSTART.md](QUICKSTART.md) - Getting started guide
- [BUILD.md](BUILD.md) - Detailed build instructions
- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture overview
- [SMART_OFFLOADING.md](SMART_OFFLOADING.md) - Offloading algorithm details
- [MULTI_WORKER.md](MULTI_WORKER.md) - Multi-worker system guide
- [PEER_TO_PEER.md](PEER_TO_PEER.md) - Peer-to-peer features guide
- [GUI_GUIDE.md](GUI_GUIDE.md) - GUI features and usage guide
- [CI_CD.md](CI_CD.md) - CI/CD workflows and build automation

