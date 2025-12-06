package com.jobs.app.data

import android.app.ActivityManager
import android.content.Context
import android.net.wifi.WifiManager
import android.os.Build
import androidx.annotation.RequiresApi
import com.jobs.app.JobsApplication
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.BufferedReader
import java.io.FileReader
import java.io.IOException

data class DeviceResources(
    val cpuUsage: Float,
    val memoryUsage: Float,
    val gpuUsage: Float,
    val networkBandwidthMbps: Float,
    val availableMemoryMB: Long,
    val totalMemoryMB: Long,
    val cpuCores: Int
)

class ResourceMonitor {
    private var isMonitoring = true
    private val context = JobsApplication.instance

    fun getCurrentResources(): DeviceResources {
        return DeviceResources(
            cpuUsage = getCpuUsage(),
            memoryUsage = getMemoryUsage(),
            gpuUsage = getGpuUsage(),
            networkBandwidthMbps = getNetworkBandwidth(),
            availableMemoryMB = getAvailableMemory(),
            totalMemoryMB = getTotalMemory(),
            cpuCores = getCpuCores()
        )
    }

    private fun getCpuUsage(): Float {
        return try {
            val reader = BufferedReader(FileReader("/proc/stat"))
            val line = reader.readLine()
            reader.close()

            val parts = line.split("\\s+".toRegex())
            if (parts.size >= 8) {
                val user = parts[1].toLong()
                val nice = parts[2].toLong()
                val system = parts[3].toLong()
                val idle = parts[4].toLong()
                val total = user + nice + system + idle
                
                if (total > 0) {
                    ((user + nice + system) * 100.0 / total).toFloat()
                } else {
                    0f
                }
            } else {
                0f
            }
        } catch (e: Exception) {
            0f
        }
    }

    private fun getMemoryUsage(): Float {
        val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val memInfo = ActivityManager.MemoryInfo()
        activityManager.getMemoryInfo(memInfo)
        
        val totalMemory = getTotalMemory()
        val usedMemory = totalMemory - (memInfo.availMem / (1024 * 1024))
        
        return if (totalMemory > 0) {
            (usedMemory * 100.0 / totalMemory).toFloat()
        } else {
            0f
        }
    }

    private fun getGpuUsage(): Float {
        // GPU usage is harder to get on Android without root or special APIs
        // This is a simplified version - in production, you might use:
        // - GPU profiling tools
        // - Vendor-specific APIs (Adreno, Mali, etc.)
        // - System properties
        
        // For now, return a placeholder that could be enhanced
        return try {
            // Attempt to read GPU frequency or usage if available
            // This is device/vendor specific
            0f // Placeholder
        } catch (e: Exception) {
            0f
        }
    }

    private fun getNetworkBandwidth(): Float {
        return try {
            val wifiManager = context.applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
            val wifiInfo = wifiManager.connectionInfo
            
            // Estimate bandwidth based on WiFi link speed
            val linkSpeed = wifiInfo.linkSpeed
            if (linkSpeed > 0) {
                // Convert Mbps (link speed is in Mbps)
                linkSpeed.toFloat()
            } else {
                0f
            }
        } catch (e: Exception) {
            0f
        }
    }

    private fun getAvailableMemory(): Long {
        val activityManager = context.getSystemService(Context.ACTIVITY_SERVICE) as ActivityManager
        val memInfo = ActivityManager.MemoryInfo()
        activityManager.getMemoryInfo(memInfo)
        return memInfo.availMem / (1024 * 1024) // Convert to MB
    }

    private fun getTotalMemory(): Long {
        return try {
            val reader = BufferedReader(FileReader("/proc/meminfo"))
            var line: String?
            while (reader.readLine().also { line = it } != null) {
                if (line?.startsWith("MemTotal:") == true) {
                    val parts = line!!.split("\\s+".toRegex())
                    if (parts.size >= 2) {
                        reader.close()
                        return parts[1].toLong() / 1024 // Convert KB to MB
                    }
                }
            }
            reader.close()
            0L
        } catch (e: Exception) {
            0L
        }
    }

    private fun getCpuCores(): Int {
        return Runtime.getRuntime().availableProcessors()
    }

    fun stop() {
        isMonitoring = false
    }
}

