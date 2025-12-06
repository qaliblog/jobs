package com.jobs.app.network

import com.jobs.app.data.Task
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import okhttp3.*
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONObject
import org.json.JSONArray
import java.io.IOException
import java.util.concurrent.TimeUnit

class ApiClient(private val serverAddress: String, private val port: Int) {
    private val client = OkHttpClient.Builder()
        .connectTimeout(10, TimeUnit.SECONDS)
        .readTimeout(30, TimeUnit.SECONDS)
        .writeTimeout(30, TimeUnit.SECONDS)
        .build()

    private val baseUrl = "http://$serverAddress:$port"

    suspend fun testConnection(): Boolean = withContext(Dispatchers.IO) {
        try {
            val request = Request.Builder()
                .url("$baseUrl/health")
                .get()
                .build()

            val response = client.newCall(request).execute()
            response.isSuccessful
        } catch (e: Exception) {
            false
        }
    }

    suspend fun submitTask(task: Task): TaskResult? = withContext(Dispatchers.IO) {
        try {
            val json = JSONObject().apply {
                put("id", task.id)
                put("type", task.type)
                put("complexity", task.complexity)
                put("dataSize", task.dataSize)
                put("requiresGpu", task.requiresGpu)
            }

            val mediaType = "application/json; charset=utf-8".toMediaType()
            val requestBody = json.toString().toRequestBody(mediaType)

            val request = Request.Builder()
                .url("$baseUrl/api/tasks")
                .post(requestBody)
                .build()

            val response = client.newCall(request).execute()
            
            if (response.isSuccessful) {
                val responseBody = response.body?.string()
                responseBody?.let { parseTaskResult(it) }
            } else {
                null
            }
        } catch (e: Exception) {
            null
        }
    }

    suspend fun getServerResources(): ServerResources? = withContext(Dispatchers.IO) {
        try {
            val request = Request.Builder()
                .url("$baseUrl/api/resources")
                .get()
                .build()

            val response = client.newCall(request).execute()
            
            if (response.isSuccessful) {
                val responseBody = response.body?.string()
                responseBody?.let { parseServerResources(it) }
            } else {
                null
            }
        } catch (e: Exception) {
            null
        }
    }
    
    suspend fun getWorkers(): List<WorkerInfo>? = withContext(Dispatchers.IO) {
        try {
            val request = Request.Builder()
                .url("$baseUrl/api/workers")
                .get()
                .build()

            val response = client.newCall(request).execute()
            
            if (response.isSuccessful) {
                val responseBody = response.body?.string()
                responseBody?.let { parseWorkers(it) }
            } else {
                null
            }
        } catch (e: Exception) {
            null
        }
    }
    
    suspend fun sendRecruitRequest(
        requesterId: String,
        requesterName: String,
        requesterAddress: String,
        requesterPort: Int
    ): Boolean = withContext(Dispatchers.IO) {
        try {
            val json = JSONObject().apply {
                put("requesterId", requesterId)
                put("requesterName", requesterName)
                put("requesterAddress", requesterAddress)
                put("requesterPort", requesterPort)
            }
            
            val mediaType = "application/json; charset=utf-8".toMediaType()
            val requestBody = json.toString().toRequestBody(mediaType)
            
            val request = Request.Builder()
                .url("$baseUrl/api/recruit")
                .post(requestBody)
                .build()
            
            val response = client.newCall(request).execute()
            response.isSuccessful
        } catch (e: Exception) {
            false
        }
    }
    
    suspend fun sendWorkRequest(
        workerId: String,
        workerType: String,
        capabilities: String
    ): Boolean = withContext(Dispatchers.IO) {
        try {
            val json = JSONObject().apply {
                put("workerId", workerId)
                put("workerType", workerType)
                put("capabilities", capabilities)
            }
            
            val mediaType = "application/json; charset=utf-8".toMediaType()
            val requestBody = json.toString().toRequestBody(mediaType)
            
            val request = Request.Builder()
                .url("$baseUrl/api/work")
                .post(requestBody)
                .build()
            
            val response = client.newCall(request).execute()
            response.isSuccessful
        } catch (e: Exception) {
            false
        }
    }

    private fun parseTaskResult(json: String): TaskResult {
        val obj = JSONObject(json)
        return TaskResult(
            taskId = obj.getString("taskId"),
            result = obj.getString("result"),
            processingTime = obj.getLong("processingTime")
        )
    }

    private fun parseServerResources(json: String): ServerResources {
        val obj = JSONObject(json)
        return ServerResources(
            cpuUsage = obj.getDouble("cpuUsage").toFloat(),
            memoryUsage = obj.getDouble("memoryUsage").toFloat(),
            gpuUsage = obj.getDouble("gpuUsage").toFloat(),
            availableMemory = obj.getLong("availableMemory"),
            totalMemory = obj.getLong("totalMemory"),
            cpuCores = obj.getInt("cpuCores")
        )
    }
    
    private fun parseWorkers(json: String): List<WorkerInfo> {
        val obj = JSONObject(json)
        val workersArray = obj.getJSONArray("workers")
        val workers = mutableListOf<WorkerInfo>()
        
        for (i in 0 until workersArray.length()) {
            val workerObj = workersArray.getJSONObject(i)
            workers.add(
                WorkerInfo(
                    id = workerObj.getString("id"),
                    type = workerObj.getString("type"),
                    address = workerObj.getString("address"),
                    port = workerObj.getInt("port"),
                    isActive = workerObj.getBoolean("isActive"),
                    cpuCores = workerObj.getInt("cpuCores"),
                    cpuUsage = workerObj.getDouble("cpuUsage").toFloat(),
                    availableMemory = workerObj.getLong("availableMemory"),
                    hasGpu = workerObj.getBoolean("hasGpu"),
                    activeTasks = workerObj.getInt("activeTasks"),
                    completedTasks = workerObj.getInt("completedTasks")
                )
            )
        }
        
        return workers
    }

    fun close() {
        client.dispatcher.executorService.shutdown()
    }
}

data class TaskResult(
    val taskId: String,
    val result: String,
    val processingTime: Long
)

data class ServerResources(
    val cpuUsage: Float,
    val memoryUsage: Float,
    val gpuUsage: Float,
    val availableMemory: Long,
    val totalMemory: Long,
    val cpuCores: Int
)

data class WorkerInfo(
    val id: String,
    val type: String,
    val address: String,
    val port: Int,
    val isActive: Boolean,
    val cpuCores: Int,
    val cpuUsage: Float,
    val availableMemory: Long,
    val hasGpu: Boolean,
    val activeTasks: Int,
    val completedTasks: Int
)

