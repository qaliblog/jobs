package com.jobs.app.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.jobs.app.data.ResourceMonitor
import com.jobs.app.data.TaskOffloader
import com.jobs.app.data.NetworkScanner
import com.jobs.app.data.DiscoveredDevice
import com.jobs.app.JobsApplication
import com.jobs.app.network.ApiClient
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.delay

data class MainUiState(
    val serverAddress: String = "192.168.1.100",
    val port: String = "8080",
    val isConnected: Boolean = false,
    val isServerMode: Boolean = false,
    val cpuUsage: Float = 0f,
    val memoryUsage: Float = 0f,
    val gpuUsage: Float = 0f,
    val networkBandwidth: Float = 0f,
    val tasksQueued: Int = 0,
    val tasksCompleted: Int = 0,
    val lastOffloadDecision: String = "No tasks yet",
    val activeWorkers: Int = 0,
    val discoveredDevices: List<DiscoveredDevice> = emptyList(),
    val isScanning: Boolean = false,
    val pendingRequests: List<ConnectionRequest> = emptyList()
)

data class ConnectionRequest(
    val id: String,
    val deviceName: String,
    val deviceAddress: String,
    val deviceType: String,
    val requestType: String, // "recruit" or "work"
    val timestamp: Long = System.currentTimeMillis()
)

class MainViewModel : ViewModel() {
    private val _uiState = MutableStateFlow(MainUiState())
    val uiState: StateFlow<MainUiState> = _uiState.asStateFlow()

    private val resourceMonitor = ResourceMonitor()
    private val taskOffloader = TaskOffloader()
    private val networkScanner = NetworkScanner(JobsApplication.instance)
    private var apiClient: ApiClient? = null

    init {
        startResourceMonitoring()
    }

    fun updateServerAddress(address: String) {
        _uiState.value = _uiState.value.copy(serverAddress = address)
    }

    fun updatePort(port: String) {
        _uiState.value = _uiState.value.copy(port = port)
    }

    fun connect() {
        viewModelScope.launch {
            try {
                val client = ApiClient(_uiState.value.serverAddress, _uiState.value.port.toInt())
                if (client.testConnection()) {
                    apiClient = client
                    _uiState.value = _uiState.value.copy(isConnected = true)
                    taskOffloader.initialize(client)
                    startWorkerMonitoring()
                } else {
                    // Handle connection failure
                }
            } catch (e: Exception) {
                // Handle error
            }
        }
    }
    
    private fun startWorkerMonitoring() {
        viewModelScope.launch {
            while (_uiState.value.isConnected) {
                try {
                    val workers = apiClient?.getWorkers()
                    _uiState.value = _uiState.value.copy(
                        activeWorkers = workers?.size ?: 0
                    )
                } catch (e: Exception) {
                    // Handle error
                }
                delay(5000) // Update every 5 seconds
            }
        }
    }

    fun disconnect() {
        apiClient?.close()
        apiClient = null
        _uiState.value = _uiState.value.copy(isConnected = false)
    }
    
    fun startNetworkScan() {
        viewModelScope.launch {
            _uiState.value = _uiState.value.copy(isScanning = true)
            val devices = networkScanner.scanNetwork(8080)
            _uiState.value = _uiState.value.copy(
                discoveredDevices = devices,
                isScanning = false
            )
        }
    }
    
    fun stopNetworkScan() {
        networkScanner.stopScanning()
        _uiState.value = _uiState.value.copy(isScanning = false)
    }
    
    fun recruitDevice(device: DiscoveredDevice) {
        viewModelScope.launch {
            try {
                val client = ApiClient(device.address, device.port)
                val success = client.sendRecruitRequest(
                    requesterId = "android-${android.os.Build.MODEL}",
                    requesterName = "Android Device",
                    requesterAddress = getLocalIpAddress(),
                    requesterPort = _uiState.value.port.toInt()
                )
                if (success) {
                    // Device accepted, now connected
                    apiClient = client
                    _uiState.value = _uiState.value.copy(
                        isConnected = true,
                        serverAddress = device.address,
                        port = device.port.toString()
                    )
                }
            } catch (e: Exception) {
                // Handle error
            }
        }
    }
    
    fun requestToWork(device: DiscoveredDevice) {
        viewModelScope.launch {
            try {
                val client = ApiClient(device.address, device.port)
                val resources = resourceMonitor.getCurrentResources()
                val success = client.sendWorkRequest(
                    workerId = "android-${android.os.Build.MODEL}",
                    workerType = "android",
                    capabilities = "cpuCores:${resources.cpuCores},availableMemory:${resources.availableMemoryMB},hasGpu:false"
                )
                if (success) {
                    apiClient = client
                    _uiState.value = _uiState.value.copy(
                        isConnected = true,
                        serverAddress = device.address,
                        port = device.port.toString()
                    )
                }
            } catch (e: Exception) {
                // Handle error
            }
        }
    }
    
    fun startServerMode() {
        // Start embedded server (would need to implement)
        _uiState.value = _uiState.value.copy(isServerMode = true)
    }
    
    fun stopServerMode() {
        _uiState.value = _uiState.value.copy(isServerMode = false)
    }
    
    private fun getLocalIpAddress(): String {
        // Get local IP address
        return try {
            val interfaces = java.net.NetworkInterface.getNetworkInterfaces()
            while (interfaces.hasMoreElements()) {
                val networkInterface = interfaces.nextElement()
                val addresses = networkInterface.inetAddresses
                while (addresses.hasMoreElements()) {
                    val address = addresses.nextElement()
                    if (!address.isLoopbackAddress && address is java.net.Inet4Address) {
                        return address.hostAddress ?: "127.0.0.1"
                    }
                }
            }
            "127.0.0.1"
        } catch (e: Exception) {
            "127.0.0.1"
        }
    }

    fun submitTestTask() {
        viewModelScope.launch {
            val task = createTestTask()
            val decision = taskOffloader.shouldOffloadTask(
                task = task,
                mobileResources = resourceMonitor.getCurrentResources(),
                networkBandwidth = _uiState.value.networkBandwidth
            )
            
            _uiState.value = _uiState.value.copy(
                lastOffloadDecision = decision.reason,
                tasksQueued = _uiState.value.tasksQueued + 1
            )

            if (decision.shouldOffload && _uiState.value.isConnected) {
                // Offload to server
                apiClient?.submitTask(task)?.let { result ->
                    _uiState.value = _uiState.value.copy(
                        tasksCompleted = _uiState.value.tasksCompleted + 1,
                        tasksQueued = _uiState.value.tasksQueued - 1
                    )
                }
            } else {
                // Process locally
                processTaskLocally(task)
                _uiState.value = _uiState.value.copy(
                    tasksCompleted = _uiState.value.tasksCompleted + 1,
                    tasksQueued = _uiState.value.tasksQueued - 1
                )
            }
        }
    }

    private fun startResourceMonitoring() {
        viewModelScope.launch {
            while (true) {
                val resources = resourceMonitor.getCurrentResources()
                _uiState.value = _uiState.value.copy(
                    cpuUsage = resources.cpuUsage,
                    memoryUsage = resources.memoryUsage,
                    gpuUsage = resources.gpuUsage,
                    networkBandwidth = resources.networkBandwidthMbps
                )
                delay(1000) // Update every second
            }
        }
    }

    private fun createTestTask(): Task {
        return Task(
            id = System.currentTimeMillis().toString(),
            type = "computation",
            complexity = (1..10).random().toFloat(),
            dataSize = (100..10000).random(),
            requiresGpu = false
        )
    }

    private suspend fun processTaskLocally(task: Task) {
        // Simulate local processing
        delay((task.complexity * 100).toLong())
    }

    override fun onCleared() {
        super.onCleared()
        resourceMonitor.stop()
        apiClient?.close()
    }
}

