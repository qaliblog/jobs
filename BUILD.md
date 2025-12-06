# Building the Jobs Application

## Prerequisites

### Android Development
- Android Studio (latest version)
- JDK 17 or higher
- Android SDK (API 26+)
- Gradle 8.2+

### C++ Backend
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.20 or higher
- OpenSSL development libraries
- pthread (usually included)

Optional for GPU support:
- CUDA Toolkit (for NVIDIA GPUs)
- OpenCL (for AMD/Intel GPUs)

## Building the Android App

1. Open the `android` directory in Android Studio
2. Sync Gradle files
3. Build the project:
   ```bash
   cd android
   ./gradlew build
   ```
4. Install on device:
   ```bash
   ./gradlew installDebug
   ```

## Building the C++ Backend

### Linux

```bash
cd backend
mkdir build && cd build
cmake ..
make
```

Or use the Makefile:
```bash
cd backend
make build
```

### Running the Backend

```bash
cd backend/build
./jobs_backend [port]
```

Default port is 8080. You can specify a different port:
```bash
./jobs_backend 9090
```

## Testing

### Test Backend Connection
```bash
curl http://localhost:8080/health
```

### Test Resource Endpoint
```bash
curl http://localhost:8080/api/resources
```

### Submit a Test Task
```bash
curl -X POST http://localhost:8080/api/tasks \
  -H "Content-Type: application/json" \
  -d '{
    "id": "test-1",
    "type": "computation",
    "complexity": 5.0,
    "dataSize": 1000,
    "requiresGpu": false
  }'
```

## Configuration

### Android App
- Default server address: `192.168.1.100:8080`
- Can be changed in the app UI

### Backend
- Default port: `8080`
- Number of worker threads: `4` (configurable in `TaskProcessor` constructor)
- Task queue timeout: `1000ms`

## Troubleshooting

### Android
- Ensure device and laptop are on the same WiFi network
- Check firewall settings on the laptop
- Verify server IP address in app settings

### Backend
- Check if port is already in use:
  ```bash
  lsof -i :8080
  ```
- Ensure OpenSSL is installed:
  ```bash
  sudo apt-get install libssl-dev  # Ubuntu/Debian
  ```
- For GPU support, install appropriate drivers and libraries

