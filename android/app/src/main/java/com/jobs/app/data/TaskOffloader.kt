package com.jobs.app.data

import com.jobs.app.network.ApiClient

data class Task(
    val id: String,
    val type: String,
    val complexity: Float, // 1-10 scale
    val dataSize: Int, // Size in KB
    val requiresGpu: Boolean
)

data class OffloadDecision(
    val shouldOffload: Boolean,
    val reason: String,
    val estimatedLocalTime: Long, // milliseconds
    val estimatedRemoteTime: Long, // milliseconds
    val transferTime: Long // milliseconds
)

class TaskOffloader {
    private var apiClient: ApiClient? = null

    fun initialize(client: ApiClient) {
        apiClient = client
    }

    fun shouldOffloadTask(
        task: Task,
        mobileResources: DeviceResources,
        networkBandwidth: Float
    ): OffloadDecision {
        // Calculate estimated processing times
        val estimatedLocalTime = estimateLocalProcessingTime(task, mobileResources)
        val transferTime = estimateTransferTime(task, networkBandwidth)
        val estimatedRemoteTime = estimateRemoteProcessingTime(task)
        val totalRemoteTime = transferTime * 2 + estimatedRemoteTime // Upload + Process + Download

        // Decision factors
        val mobileCpuLoad = mobileResources.cpuUsage
        val mobileMemoryLoad = mobileResources.memoryUsage
        val networkAvailable = networkBandwidth > 1.0f // At least 1 Mbps

        // Smart decision logic
        var shouldOffload = false
        var reason = ""

        // Factor 1: Network availability
        if (!networkAvailable) {
            return OffloadDecision(
                shouldOffload = false,
                reason = "Network unavailable (${String.format("%.2f", networkBandwidth)} Mbps)",
                estimatedLocalTime,
                estimatedRemoteTime,
                transferTime
            )
        }

        // Factor 2: Task complexity vs network overhead
        val networkOverheadRatio = (transferTime * 2) / estimatedLocalTime.toFloat()
        if (networkOverheadRatio > 0.5f && task.complexity < 5.0f) {
            return OffloadDecision(
                shouldOffload = false,
                reason = "Network overhead too high for simple task",
                estimatedLocalTime,
                estimatedRemoteTime,
                transferTime
            )
        }

        // Factor 3: Mobile device resource availability
        val mobileResourceScore = (100f - mobileCpuLoad) * 0.5f + (100f - mobileMemoryLoad) * 0.3f
        val shouldUseMobile = mobileResourceScore > 30f && estimatedLocalTime < totalRemoteTime

        // Factor 4: High complexity tasks favor offloading
        val complexityFactor = if (task.complexity > 7.0f) 1.5f else 1.0f
        val adjustedRemoteTime = totalRemoteTime / complexityFactor

        // Factor 5: GPU requirements
        if (task.requiresGpu && mobileResources.gpuUsage > 80f) {
            shouldOffload = true
            reason = "GPU overloaded, offloading GPU task"
        } else if (estimatedLocalTime > adjustedRemoteTime * 1.2f) {
            // Remote is significantly faster (20% threshold)
            shouldOffload = true
            reason = "Remote processing faster (${String.format("%.1f", estimatedLocalTime)}ms vs ${String.format("%.1f", adjustedRemoteTime)}ms)"
        } else if (mobileCpuLoad > 80f || mobileMemoryLoad > 85f) {
            // Mobile device is overloaded
            shouldOffload = true
            reason = "Mobile device overloaded (CPU: ${String.format("%.1f", mobileCpuLoad)}%, Mem: ${String.format("%.1f", mobileMemoryLoad)}%)"
        } else if (shouldUseMobile) {
            shouldOffload = false
            reason = "Processing locally (faster and resources available)"
        } else {
            shouldOffload = false
            reason = "Local processing preferred"
        }

        return OffloadDecision(
            shouldOffload = shouldOffload,
            reason = reason,
            estimatedLocalTime = estimatedLocalTime,
            estimatedRemoteTime = estimatedRemoteTime,
            transferTime = transferTime
        )
    }

    private fun estimateLocalProcessingTime(task: Task, resources: DeviceResources): Long {
        // Base time based on complexity
        val baseTime = task.complexity * 100 // milliseconds per complexity unit
        
        // Adjust based on CPU availability
        val cpuFactor = if (resources.cpuUsage > 80f) 2.0f else 1.0f
        val memoryFactor = if (resources.memoryUsage > 85f) 1.5f else 1.0f
        
        // Adjust for number of cores (more cores = faster)
        val coreFactor = 1.0f / (1.0f + resources.cpuCores * 0.1f)
        
        return (baseTime * cpuFactor * memoryFactor * coreFactor).toLong()
    }

    private fun estimateRemoteProcessingTime(task: Task): Long {
        // Assume remote has better resources (10x faster for high complexity)
        val baseTime = task.complexity * 10 // Remote is much faster
        return baseTime.toLong()
    }

    private fun estimateTransferTime(task: Task, bandwidthMbps: Float): Long {
        if (bandwidthMbps <= 0) return Long.MAX_VALUE
        
        // Convert data size from KB to bits
        val dataSizeBits = task.dataSize * 8 * 1024
        // Convert bandwidth from Mbps to bps
        val bandwidthBps = bandwidthMbps * 1024 * 1024
        
        // Time in milliseconds
        val timeSeconds = dataSizeBits / bandwidthBps
        return (timeSeconds * 1000).toLong()
    }
}

