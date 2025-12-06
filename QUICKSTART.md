# Quick Start Guide

## Prerequisites

- Android device (Poco X6 Pro or similar) with Android 8.0+
- Laptop/desktop with Linux (Ubuntu/Debian recommended)
- Both devices on the same WiFi network

## Step 1: Build and Run the Backend

```bash
cd backend

# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y build-essential cmake libssl-dev

# Build
mkdir build && cd build
cmake ..
make

# Run (default port 8080)
./jobs_backend
```

The server will start and display:
```
Starting Jobs Backend Server on port 8080
Server listening on port 8080
Task processor started with 4 threads
```

## Step 2: Find Your Laptop's IP Address

On Linux:
```bash
ip addr show | grep "inet " | grep -v 127.0.0.1
```

Or:
```bash
hostname -I
```

Note the IP address (e.g., `192.168.1.100`)

## Step 3: Build and Install Android App

### Option A: Using Android Studio
1. Open Android Studio
2. File → Open → Select `android` directory
3. Wait for Gradle sync
4. Connect your Android device via USB
5. Enable USB debugging on device
6. Click Run (green play button)

### Option B: Using Command Line
```bash
cd android
./gradlew installDebug
```

## Step 4: Configure Connection

1. Open the Jobs app on your Android device
2. Enter your laptop's IP address in "Server IP" field
3. Enter port `8080` (or your custom port)
4. Tap "Connect"
5. Status should show "Connected"

## Step 5: Test the System

1. Tap "Submit Test Task" button
2. Watch the resource monitors update
3. Check the "Last Decision" field to see offloading decisions
4. Monitor task statistics

## Troubleshooting

### Cannot Connect to Server
- Verify both devices are on the same WiFi network
- Check firewall: `sudo ufw allow 8080`
- Verify server is running: `curl http://localhost:8080/health`
- Try pinging laptop from phone

### App Crashes
- Check logcat: `adb logcat | grep Jobs`
- Verify minimum SDK (Android 8.0+)
- Ensure all permissions are granted

### Backend Won't Start
- Check if port is in use: `lsof -i :8080`
- Verify OpenSSL is installed: `dpkg -l | grep libssl`
- Check build errors in `build/` directory

### Tasks Not Processing
- Verify backend is receiving requests (check server console)
- Check network connectivity
- Verify task queue is not full

## Testing Offloading Logic

To test different scenarios:

1. **Force Local Processing**: Disconnect from WiFi or use very low bandwidth
2. **Force Offloading**: Submit high-complexity tasks (complexity > 7)
3. **Test GPU Tasks**: Submit tasks with `requiresGpu: true`
4. **Test Resource Overload**: Run other apps to increase CPU/memory usage

## Next Steps

- Review `SMART_OFFLOADING.md` for algorithm details
- Check `BUILD.md` for advanced build options
- Customize task types in `TaskOffloader.kt`
- Add GPU support in `gpu_accelerator.cpp`

## Performance Tips

1. **Network**: Use 5GHz WiFi for better bandwidth
2. **Backend**: Increase worker threads for more parallelism
3. **Mobile**: Close background apps to free resources
4. **Tasks**: Batch small tasks to reduce network overhead

