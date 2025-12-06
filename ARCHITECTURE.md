# Jobs Architecture

## System Overview

Jobs is a distributed computing platform that enables Android mobile devices to offload compute-intensive tasks to a high-performance C++ backend running on a laptop/desktop.

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    Android Client (Kotlin)                   │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │     UI       │  │   Resource   │  │    Task      │     │
│  │  (Compose)   │→ │   Monitor    │→ │  Offloader   │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│                              │                             │
│                              ↓                             │
│                    ┌──────────────┐                        │
│                    │  API Client  │                        │
│                    │   (HTTP)     │                        │
│                    └──────────────┘                        │
└────────────────────────────┬──────────────────────────────┘
                              │
                              │ WiFi Network
                              │
┌─────────────────────────────┴──────────────────────────────┐
│                  C++ Backend Server                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │   HTTP       │  │    Task      │  │   Resource   │     │
│  │   Server     │→ │    Queue     │→ │   Monitor    │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│                              │                             │
│                              ↓                             │
│                    ┌──────────────┐                        │
│                    │    Task      │                        │
│                    │  Processor   │                        │
│                    │ (Multi-thread)│                        │
│                    └──────────────┘                        │
│                              │                             │
│                              ↓                             │
│                    ┌──────────────┐                        │
│                    │     GPU      │                        │
│                    │  Accelerator │                        │
│                    └──────────────┘                        │
└─────────────────────────────────────────────────────────────┘
```

## Components

### Android Client

#### UI Layer (MainActivity.kt)
- Jetpack Compose interface
- Real-time resource monitoring display
- Connection management
- Task submission interface

#### ViewModel (MainViewModel.kt)
- State management
- Business logic coordination
- Connection lifecycle management

#### Resource Monitor (ResourceMonitor.kt)
- CPU usage monitoring (via /proc/stat)
- Memory usage tracking
- GPU usage estimation
- Network bandwidth detection
- Updates every second

#### Task Offloader (TaskOffloader.kt)
- Smart decision algorithm
- Time estimation (local vs remote)
- Network overhead calculation
- Resource-aware offloading

#### API Client (ApiClient.kt)
- HTTP communication with backend
- Task submission
- Resource querying
- Connection testing

### C++ Backend

#### Server (server.cpp)
- HTTP server implementation
- Request routing
- JSON parsing (simple regex-based)
- Connection handling

#### Task Queue (task_queue.cpp)
- Thread-safe queue
- Blocking pop with timeout
- Size tracking

#### Task Processor (task_processor.cpp)
- Multi-threaded worker pool
- Task execution
- CPU-intensive computation
- GPU task routing
- Performance statistics

#### Resource Monitor (resource_monitor.cpp)
- System resource tracking
- CPU usage (via /proc/stat)
- Memory usage (via sysinfo)
- GPU usage (placeholder for vendor APIs)
- JSON serialization

#### GPU Accelerator (gpu_accelerator.cpp)
- GPU initialization
- CUDA/OpenCL integration points
- GPU computation execution
- Fallback to CPU

## Communication Protocol

### HTTP Endpoints

#### GET /health
Health check endpoint
```json
Response: {"status":"ok"}
```

#### GET /api/resources
Get server resource status
```json
Response: {
  "cpuUsage": 45.2,
  "memoryUsage": 62.1,
  "gpuUsage": 0.0,
  "availableMemory": 8192,
  "totalMemory": 16384,
  "cpuCores": 8
}
```

#### POST /api/tasks
Submit a task for processing
```json
Request: {
  "id": "task-123",
  "type": "computation",
  "complexity": 7.5,
  "dataSize": 1500,
  "requiresGpu": false
}

Response: {
  "taskId": "task-123",
  "status": "queued"
}
```

## Data Flow

### Task Submission Flow

1. User submits task via UI
2. ViewModel creates Task object
3. TaskOffloader evaluates offloading decision
4. If offload: API Client sends HTTP POST to backend
5. Backend Server receives request, parses JSON
6. Task added to TaskQueue
7. TaskProcessor worker thread picks up task
8. Task processed (CPU or GPU)
9. Result returned (currently queued, could be async callback)

### Resource Monitoring Flow

1. ResourceMonitor polls system every second
2. Updates DeviceResources object
3. ViewModel collects updates
4. UI displays real-time metrics
5. TaskOffloader uses resources for decisions

## Threading Model

### Android Client
- Main thread: UI updates
- Coroutine scope: Network I/O, resource monitoring
- Background: Task processing (if local)

### C++ Backend
- Main thread: Server accept loop
- Worker threads: Task processing (configurable, default 4)
- Each worker: Processes tasks from queue independently

## Performance Optimizations

1. **Multithreading**: Backend uses worker pool for parallel processing
2. **Smart Offloading**: Avoids unnecessary network transfers
3. **Resource Awareness**: Adapts to current system load
4. **Network Optimization**: Considers bandwidth in decisions
5. **GPU Acceleration**: Routes GPU tasks to appropriate hardware

## Security Considerations

Current implementation:
- HTTP (not HTTPS) - suitable for local network
- No authentication - assumes trusted local network

Production enhancements:
- TLS/SSL encryption
- Authentication tokens
- Input validation
- Rate limiting
- Task size limits

## Scalability

### Current Limitations
- Single backend server
- Synchronous HTTP requests
- No task result callback mechanism

### Future Enhancements
- Multiple backend support
- WebSocket for real-time updates
- Task result streaming
- Load balancing
- Distributed task queue

## Error Handling

### Android Client
- Network errors: Caught in ApiClient, returns null
- Connection failures: Displayed in UI
- Resource monitoring: Graceful degradation

### C++ Backend
- Socket errors: Logged, continue accepting
- Task processing errors: Logged, task marked failed
- Resource monitoring: Returns 0 on failure

## Testing Strategy

### Unit Tests
- TaskOffloader decision logic
- ResourceMonitor calculations
- Time estimation functions

### Integration Tests
- End-to-end task submission
- Network failure scenarios
- Resource overload scenarios

### Performance Tests
- Concurrent task processing
- Large task handling
- Network bandwidth impact

