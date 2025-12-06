package com.jobs.app.data

import android.content.Context
import android.net.wifi.WifiManager
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.coroutines.async
import kotlinx.coroutines.awaitAll
import kotlinx.coroutines.async
import kotlinx.coroutines.awaitAll
import kotlinx.coroutines.CoroutineScope
import java.net.InetAddress
import java.net.Socket
import java.net.SocketTimeoutException

data class DiscoveredDevice(
    val id: String,
    val name: String,
    val address: String,
    val port: Int,
    val type: String, // "android", "linux", "windows"
    val isServer: Boolean,
    val isWorker: Boolean
)

class NetworkScanner(private val context: Context) {
    private val discoveredDevices = mutableListOf<DiscoveredDevice>()
    private var isScanning = false
    
    suspend fun scanNetwork(port: Int = 8080): List<DiscoveredDevice> = withContext(Dispatchers.IO) {
        discoveredDevices.clear()
        isScanning = true
        
        try {
            val wifiManager = context.applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
            val wifiInfo = wifiManager.connectionInfo
            val ipAddress = wifiInfo.ipAddress
            
            // Get network prefix
            val networkPrefix = String.format(
                "%d.%d.%d.",
                (ipAddress and 0xff),
                (ipAddress shr 8 and 0xff),
                (ipAddress shr 16 and 0xff)
            )
            
            // Parallel scanning using coroutines (batch of 50 at a time)
            val batchSize = 50
            (1..254).chunked(batchSize).forEach { batch ->
                if (!isScanning) return@forEach
                
                val jobs = batch.map { i ->
                    async {
                        if (!isScanning) return@async
                        
                        val ip = "$networkPrefix$i"
                        try {
                            // Fast connection attempt
                            val socket = Socket()
                            socket.soTimeout = 100 // 100ms timeout
                            socket.connect(java.net.InetSocketAddress(ip, port), 100)
                            
                            // Send discovery request
                            val request = "GET /api/discover HTTP/1.1\r\nHost: $ip:$port\r\n\r\n"
                            socket.getOutputStream().write(request.toByteArray())
                            
                            // Read response with timeout
                            val response = socket.getInputStream().bufferedReader().readText()
                            parseDiscoveryResponse(response, ip, port)
                            
                            socket.close()
                        } catch (e: SocketTimeoutException) {
                            // Timeout, skip
                        } catch (e: Exception) {
                            // Connection failed, skip
                        }
                    }
                }
                
                // Wait for batch to complete
                jobs.awaitAll()
            }
        } catch (e: Exception) {
            // Handle error
        }
        
        isScanning = false
        discoveredDevices.toList()
    }
    
    private fun parseDiscoveryResponse(response: String, address: String, port: Int) {
        try {
            val jsonStart = response.indexOf("{")
            if (jsonStart == -1) return
            
            val json = response.substring(jsonStart)
            val device = parseDeviceFromJson(json, address, port)
            if (device != null) {
                synchronized(discoveredDevices) {
                    if (!discoveredDevices.any { it.address == address }) {
                        discoveredDevices.add(device)
                    }
                }
            }
        } catch (e: Exception) {
            // Parse error
        }
    }
    
    private fun parseDeviceFromJson(json: String, address: String, port: Int): DiscoveredDevice? {
        return try {
            val id = extractJsonValue(json, "id") ?: "unknown"
            val name = extractJsonValue(json, "name") ?: "Unknown Device"
            val type = extractJsonValue(json, "type") ?: "unknown"
            val isServer = json.contains("\"isServer\":true")
            val isWorker = json.contains("\"isWorker\":true")
            
            DiscoveredDevice(id, name, address, port, type, isServer, isWorker)
        } catch (e: Exception) {
            null
        }
    }
    
    private fun extractJsonValue(json: String, key: String): String? {
        val pattern = "\"$key\"\\s*:\\s*\"([^\"]+)\""
        val regex = Regex(pattern)
        return regex.find(json)?.groupValues?.get(1)
    }
    
    fun stopScanning() {
        isScanning = false
    }
    
    fun getDiscoveredDevices(): List<DiscoveredDevice> {
        return synchronized(discoveredDevices) {
            discoveredDevices.toList()
        }
    }
}

