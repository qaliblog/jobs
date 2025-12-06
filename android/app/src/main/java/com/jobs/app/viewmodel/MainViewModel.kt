package com.jobs.app.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.jobs.app.data.ResourceMonitor
import com.jobs.app.data.TaskOffloader
import com.jobs.app.data.Task
import android.graphics.Bitmap
import com.google.zxing.BarcodeFormat
import com.google.zxing.EncodeHintType
import com.google.zxing.qrcode.QRCodeWriter
import android.graphics.Color
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
    val pendingRequests: List<ConnectionRequest> = emptyList(),
    val qrCodeVisible: Boolean = false
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
                    // Send work request to register as worker
                    val resources = resourceMonitor.getCurrentResources()
                    val workerId = "android-${android.os.Build.MODEL}-${System.currentTimeMillis()}"
                    val capabilities = "cpuCores:${resources.cpuCores},availableMemory:${resources.availableMemoryMB},hasGpu:false"
                    
                    val workRequestSent = client.sendWorkRequest(
                        workerId = workerId,
                        workerType = "android",
                        capabilities = capabilities
                    )
                    
                    if (workRequestSent) {
                        apiClient = client
                        _uiState.value = _uiState.value.copy(isConnected = true)
                        taskOffloader.initialize(client)
                        startWorkerMonitoring()
                    } else {
                        // Handle work request failure
                    }
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
    
    fun refreshRequests() {
        viewModelScope.launch {
            if (_uiState.value.isServerMode) {
                // Fetch pending requests from server
                apiClient?.getPendingRequests()?.let { requests ->
                    _uiState.value = _uiState.value.copy(pendingRequests = requests)
                }
            }
        }
    }
    
    fun acceptRequest(request: ConnectionRequest) {
        viewModelScope.launch {
            apiClient?.acceptRequest(request.id)?.let { success ->
                if (success) {
                    // Remove from list
                    val updated = _uiState.value.pendingRequests.filter { it.id != request.id }
                    _uiState.value = _uiState.value.copy(pendingRequests = updated)
                }
            }
        }
    }
    
    fun rejectRequest(request: ConnectionRequest) {
        viewModelScope.launch {
            apiClient?.rejectRequest(request.id)?.let { success ->
                if (success) {
                    // Remove from list
                    val updated = _uiState.value.pendingRequests.filter { it.id != request.id }
                    _uiState.value = _uiState.value.copy(pendingRequests = updated)
                }
            }
        }
    }
    
    fun startServerMode() {
        // Start embedded server (would need to implement)
        _uiState.value = _uiState.value.copy(isServerMode = true)
        // Start polling for requests
        viewModelScope.launch {
            while (_uiState.value.isServerMode) {
                refreshRequests()
                delay(5000) // Refresh every 5 seconds
            }
        }
    }
    
    fun stopServerMode() {
        _uiState.value = _uiState.value.copy(isServerMode = false)
    }
    
    fun showQRCode() {
        _uiState.value = _uiState.value.copy(qrCodeVisible = !_uiState.value.qrCodeVisible)
    }
    
    fun scanQRCode() {
        // Launch QR code scanner
        // This will be handled by MainActivity using CameraX and ZXing
        _uiState.value = _uiState.value.copy(qrCodeVisible = false) // Hide QR code if showing
    }
    
    fun onQRCodeScanned(connectionInfo: String) {
        // Parse connection info from QR code (format: "IP:Port")
        val parts = connectionInfo.split(":")
        if (parts.size == 2) {
            val ip = parts[0]
            val port = parts[1]
            _uiState.value = _uiState.value.copy(
                serverAddress = ip,
                port = port
            )
            // Automatically connect after scanning
            connect()
        }
    }
    
    fun getQRCodeBitmap(): Bitmap? {
        // QR code contains the receiver's (server's) IP address and port
        // This is the device showing the QR code - it will be the server
        val receiverIp = getLocalIpAddress()
        val receiverPort = _uiState.value.port
        val connectionInfo = "$receiverIp:$receiverPort"
        
        return try {
            val writer = QRCodeWriter()
            val hints = hashMapOf<EncodeHintType, Any>().apply {
                put(EncodeHintType.CHARACTER_SET, "UTF-8")
                put(EncodeHintType.MARGIN, 1)
            }
            val bitMatrix = writer.encode(connectionInfo, BarcodeFormat.QR_CODE, 512, 512, hints)
            val width = bitMatrix.width
            val height = bitMatrix.height
            val bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.RGB_565)
            
            for (x in 0 until width) {
                for (y in 0 until height) {
                    bitmap.setPixel(x, y, if (bitMatrix[x, y]) Color.BLACK else Color.WHITE)
                }
            }
            bitmap
        } catch (e: Exception) {
            null
        }
    }
    
    fun getLocalIpAddress(): String {
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

