# Peer-to-Peer Distributed Computing

## Overview

The Jobs system now supports a fully peer-to-peer architecture where any device (Android, Linux, or Windows) can:
- **Act as a coordinator/server** - Recruit other devices to work for them
- **Act as a worker** - Work for other devices
- **Discover devices** - Scan the network to find other Jobs-enabled devices
- **Send requests** - Request to recruit or work for other devices
- **Accept requests** - Accept or reject connection requests

## Features

### Network Discovery
- Automatic network scanning to find other Jobs devices
- Shows device type (Android/Linux/Windows), capabilities, and status
- Real-time device list updates

### Bidirectional Connections
- **Recruit**: Request another device to work for you
- **Work For**: Request to work for another device
- Both operations require acceptance from the target device

### Multi-Device Support
- Android devices can act as servers or workers
- Linux machines can act as servers or workers
- Windows machines can act as servers or workers
- Any combination of devices can form a computing cluster

## Usage

### Android App

1. **Start Network Scan**
   - Tap "Scan Network" button
   - Wait for devices to be discovered
   - View list of discovered devices

2. **Recruit a Device**
   - Find a device in the list
   - Tap "Recruit" button
   - Device will receive a request and can accept/reject

3. **Work For a Device**
   - Find a device in the list
   - Tap "Work For" button
   - Your device will register as a worker for that device

4. **Server Mode**
   - Toggle "Server Mode" switch
   - Your device can now accept recruit/work requests
   - Other devices can discover and connect to you

### Linux/Windows Worker

1. **Interactive Mode**
   ```bash
   ./jobs_worker
   ```
   - Choose option 2 to scan network
   - Select a device from the list
   - Connect automatically

2. **Direct Connection**
   ```bash
   ./jobs_worker <server-ip> <port> <type>
   ```

3. **Accept Requests**
   - When running, the worker can accept recruit/work requests
   - Prompts user for acceptance (can be automated)

## API Endpoints

### Discovery
```
GET /api/discover
```
Returns device information for network discovery.

### Recruit Request
```
POST /api/recruit
{
  "requesterId": "device-1",
  "requesterName": "My Device",
  "requesterAddress": "192.168.1.100",
  "requesterPort": 8080
}
```
Request another device to work for you.

### Work Request
```
POST /api/work
{
  "workerId": "worker-1",
  "workerType": "android",
  "capabilities": "cpuCores:8,availableMemory:16384,hasGpu:false"
}
```
Request to work for another device.

## Architecture

```
┌─────────────┐      ┌─────────────┐      ┌─────────────┐
│   Android   │◄────►│    Linux    │◄────►│   Windows   │
│   Device    │      │   Worker    │      │   Worker    │
└─────────────┘      └─────────────┘      └─────────────┘
      ▲                     ▲                     ▲
      │                     │                     │
      └─────────────────────┴─────────────────────┘
                    Network Discovery
```

All devices can:
- Discover each other
- Send recruit/work requests
- Accept/reject requests
- Form ad-hoc computing clusters

## Request Flow

### Recruit Flow
1. Device A scans network, finds Device B
2. Device A sends recruit request to Device B
3. Device B receives request, user accepts/rejects
4. If accepted, Device B registers as worker for Device A
5. Device A can now assign tasks to Device B

### Work For Flow
1. Device A scans network, finds Device B (server)
2. Device A sends work request to Device B
3. Device B receives request, auto-accepts or prompts user
4. Device A registers as worker for Device B
5. Device B can assign tasks to Device A

## Benefits

1. **Flexibility**: Any device can be coordinator or worker
2. **Scalability**: Easy to add/remove devices from cluster
3. **Discovery**: Automatic network scanning
4. **User Control**: Users can accept/reject connections
5. **Heterogeneous**: Mix different device types seamlessly

## Security Considerations

- All connections are on local network (assumed trusted)
- Users can accept/reject requests
- No authentication (suitable for local network)
- For production: Add authentication tokens, encryption

## Future Enhancements

- Automatic acceptance rules
- Device groups/namespaces
- Persistent connections
- WebSocket for real-time updates
- Encrypted communication
- Device authentication

