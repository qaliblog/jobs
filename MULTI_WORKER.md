# Multi-Worker System

## Overview

The Jobs system now supports multiple workers (Android devices, Linux machines, Windows machines) that can all participate in distributed task processing. The server acts as a coordinator, distributing tasks across available workers based on their capabilities and current load.

## Architecture

```
                    ┌─────────────────┐
                    │  Server/Coordinator │
                    │   (C++ Backend)    │
                    └─────────┬─────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        │                     │                     │
   ┌────▼────┐          ┌──────▼──────┐        ┌─────▼─────┐
   │ Android │          │   Linux    │        │  Windows  │
   │ Device  │          │   Worker   │        │  Worker   │
   │  (App)  │          │  (Client)  │        │  (Client) │
   └─────────┘          └────────────┘        └───────────┘
```

## Components

### Server (Coordinator)
- **Worker Manager**: Tracks all registered workers
- **Task Queue**: Central queue for all tasks
- **Load Balancer**: Selects best worker for each task
- **Health Monitor**: Tracks worker heartbeats and removes inactive workers

### Workers
- **Android App**: Can submit tasks and optionally act as worker
- **Linux Worker**: Standalone C++ client for Linux systems
- **Windows Worker**: Standalone C++ client for Windows systems

## Worker Registration

Workers register with the server by sending their capabilities:

```json
POST /api/workers/register
{
  "type": "linux",
  "port": 0,
  "capabilities": "cpuCores:8,availableMemory:16384,hasGpu:true"
}
```

Response:
```json
{
  "workerId": "worker-1",
  "status": "registered"
}
```

## Worker Heartbeat

Workers send periodic heartbeats (every 10 seconds) to indicate they're alive:

```json
POST /api/workers/heartbeat
{
  "workerId": "worker-1",
  "cpuUsage": 45.2,
  "availableMemory": 8192
}
```

Workers that don't send heartbeats for 30 seconds are automatically removed.

## Task Distribution

When a task is submitted:

1. Server evaluates all available workers
2. Calculates a score for each worker based on:
   - CPU usage (lower is better)
   - Available memory (higher is better)
   - Active task count (lower is better)
   - GPU availability (for GPU tasks)
   - CPU cores (more is better)
3. Selects the worker with the highest score
4. Assigns task to that worker

## Worker Task Processing

Workers poll the server for tasks:

```http
GET /api/workers/task?workerId=worker-1
```

Response (if task available):
```json
{
  "taskId": "task-123",
  "type": "computation",
  "complexity": 7.5,
  "dataSize": 1500,
  "requiresGpu": false,
  "data": "..."
}
```

Response (if no task):
```
HTTP/1.1 204 No Content
```

After processing, worker sends completion:

```json
POST /api/workers/complete
{
  "workerId": "worker-1",
  "taskId": "task-123",
  "result": "result:42.5"
}
```

## Building Workers

### Linux Worker
```bash
cd worker_client
mkdir build && cd build
cmake ..
make
./jobs_worker <server-ip> <server-port> linux
```

### Windows Worker
```bash
cd worker_client
mkdir build && cd build
cmake ..
# Build with Visual Studio or MinGW
jobs_worker.exe <server-ip> <server-port> windows
```

## Usage Example

1. Start the server:
   ```bash
   cd backend/build
   ./jobs_backend 8080
   ```

2. Start Linux worker:
   ```bash
   cd worker_client/build
   ./jobs_worker localhost 8080 linux
   ```

3. Start Windows worker (on Windows machine):
   ```cmd
   jobs_worker.exe 192.168.1.100 8080 windows
   ```

4. Launch Android app and connect to server

5. Submit tasks - they will be distributed across all available workers

## Worker Selection Algorithm

The server uses a scoring system to select workers:

```cpp
score = 100.0
score -= cpuUsage * 0.5              // Penalize high CPU
score -= activeTasks * 10.0           // Penalize busy workers
score += min(availableMemory/1024, 50) // Reward memory
score += cpuCores * 2.0               // Reward cores
if (task.requiresGpu && worker.hasGpu)
    score += 30.0                     // Reward GPU
if (complexity > 7.0 && (linux || windows))
    score += 20.0                     // Reward desktop for complex tasks
```

## Monitoring

### List All Workers
```bash
curl http://localhost:8080/api/workers
```

Response:
```json
{
  "workers": [
    {
      "id": "worker-1",
      "type": "linux",
      "address": "192.168.1.101",
      "port": 0,
      "isActive": true,
      "cpuCores": 8,
      "cpuUsage": 45.2,
      "availableMemory": 8192,
      "hasGpu": true,
      "activeTasks": 2,
      "completedTasks": 150
    }
  ],
  "total": 1
}
```

## Benefits

1. **Scalability**: Add more workers to increase processing capacity
2. **Load Distribution**: Tasks automatically distributed based on worker capabilities
3. **Fault Tolerance**: Inactive workers are automatically removed
4. **Resource Optimization**: Tasks routed to best-suited workers
5. **Heterogeneous Support**: Mix Android, Linux, and Windows workers

## Future Enhancements

- WebSocket for real-time task assignment
- Task result callbacks to submitting clients
- Worker priority levels
- Task dependencies and scheduling
- Distributed GPU computation across multiple GPUs

