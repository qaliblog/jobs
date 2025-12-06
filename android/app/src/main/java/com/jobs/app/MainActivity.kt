package com.jobs.app

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.jobs.app.ui.theme.JobsTheme
import com.jobs.app.viewmodel.MainViewModel
import com.jobs.app.viewmodel.ConnectionRequest
import androidx.compose.material3.Switch
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Close
import androidx.compose.ui.graphics.asImageBitmap
import android.graphics.Bitmap
import androidx.compose.foundation.Image

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            JobsTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    MainScreen()
                }
            }
        }
    }
}

@Composable
fun MainScreen(viewModel: MainViewModel = viewModel()) {
    val uiState by viewModel.uiState.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Text(
            text = "Jobs - Distributed Computing",
            style = MaterialTheme.typography.headlineLarge,
            color = MaterialTheme.colorScheme.primary
        )

        // QR Code Connection Section - Moved to top for visibility
        Card(
            modifier = Modifier.fillMaxWidth(),
            elevation = CardDefaults.cardElevation(defaultElevation = 6.dp),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.primaryContainer
            )
        ) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = "📱 QR Code Connection",
                    style = MaterialTheme.typography.titleLarge,
                    color = MaterialTheme.colorScheme.onPrimaryContainer
                )
                
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Button(
                        onClick = { viewModel.showQRCode() },
                        modifier = Modifier.weight(1f)
                    ) {
                        Text("Show My QR Code")
                    }
                    Button(
                        onClick = { viewModel.scanQRCode() },
                        modifier = Modifier.weight(1f)
                    ) {
                        Text("Scan QR Code")
                    }
                }
                
                if (uiState.qrCodeVisible) {
                    // Show QR code
                    viewModel.getQRCodeBitmap()?.let { bitmap ->
                        Image(
                            bitmap = bitmap.asImageBitmap(),
                            contentDescription = "QR Code",
                            modifier = Modifier.size(250.dp)
                        )
                    }
                    Text(
                        text = "Scan this QR code to connect to this device",
                        style = MaterialTheme.typography.bodyMedium,
                        modifier = Modifier.padding(top = 8.dp)
                    )
                    Text(
                        text = "Server Address: ${viewModel.getLocalIpAddress()}:${uiState.port}",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    Text(
                        text = "This device will be the server",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.primary
                    )
                }
            }
        }

        // Connection Section
        Card(
            modifier = Modifier.fillMaxWidth(),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Text(
                    text = "Connection",
                    style = MaterialTheme.typography.titleMedium
                )
                OutlinedTextField(
                    value = uiState.serverAddress,
                    onValueChange = viewModel::updateServerAddress,
                    label = { Text("Server IP") },
                    modifier = Modifier.fillMaxWidth()
                )
                OutlinedTextField(
                    value = uiState.port,
                    onValueChange = viewModel::updatePort,
                    label = { Text("Port") },
                    modifier = Modifier.fillMaxWidth()
                )
                Button(
                    onClick = {
                        if (uiState.isConnected) {
                            viewModel.disconnect()
                        } else {
                            viewModel.connect()
                        }
                    },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text(if (uiState.isConnected) "Disconnect" else "Connect")
                }
                Text(
                    text = "Status: ${if (uiState.isConnected) "Connected" else "Disconnected"}",
                    style = MaterialTheme.typography.bodyMedium
                )
            }
        }

        // Resource Monitoring Section
        Card(
            modifier = Modifier.fillMaxWidth(),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Text(
                    text = "Device Resources",
                    style = MaterialTheme.typography.titleMedium
                )
                ResourceBar("CPU", uiState.cpuUsage)
                ResourceBar("Memory", uiState.memoryUsage)
                ResourceBar("GPU", uiState.gpuUsage)
                Text(
                    text = "Network: ${String.format("%.2f", uiState.networkBandwidth)} Mbps",
                    style = MaterialTheme.typography.bodyMedium
                )
            }
        }

        // Task Statistics Section
        Card(
            modifier = Modifier.fillMaxWidth(),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Text(
                    text = "Task Statistics",
                    style = MaterialTheme.typography.titleMedium
                )
                Text(
                    text = "Queued: ${uiState.tasksQueued}",
                    style = MaterialTheme.typography.bodyMedium
                )
                Text(
                    text = "Completed: ${uiState.tasksCompleted}",
                    style = MaterialTheme.typography.bodyMedium
                )
                Text(
                    text = "Last Decision: ${uiState.lastOffloadDecision}",
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.primary
                )
                Text(
                    text = "Active Workers: ${uiState.activeWorkers}",
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.secondary
                )
            }
        }

        // QR Code Connection Section
        Card(
            modifier = Modifier.fillMaxWidth(),
            elevation = CardDefaults.cardElevation(defaultElevation = 6.dp),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.primaryContainer
            )
        ) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = "📱 QR Code Connection",
                    style = MaterialTheme.typography.titleLarge,
                    color = MaterialTheme.colorScheme.onPrimaryContainer
                )
                
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Button(
                        onClick = { viewModel.showQRCode() },
                        modifier = Modifier.weight(1f)
                    ) {
                        Text("Show My QR Code")
                    }
                    Button(
                        onClick = { viewModel.scanQRCode() },
                        modifier = Modifier.weight(1f)
                    ) {
                        Text("Scan QR Code")
                    }
                }
                
                if (uiState.qrCodeVisible) {
                    // Show QR code
                    viewModel.getQRCodeBitmap()?.let { bitmap ->
                        Image(
                            bitmap = bitmap.asImageBitmap(),
                            contentDescription = "QR Code",
                            modifier = Modifier.size(250.dp)
                        )
                    }
                    Text(
                        text = "Scan this QR code to connect to this device",
                        style = MaterialTheme.typography.bodyMedium,
                        modifier = Modifier.padding(top = 8.dp)
                    )
                    Text(
                        text = "Server Address: ${viewModel.getLocalIpAddress()}:${uiState.port}",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    Text(
                        text = "This device will be the server",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.primary
                    )
                }
            }
        }
        
        // Recruit Requests Section
        Card(
            modifier = Modifier.fillMaxWidth(),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp),
            colors = CardDefaults.cardColors(
                containerColor = MaterialTheme.colorScheme.secondaryContainer
            )
        ) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = "📥 Recruit Requests",
                        style = MaterialTheme.typography.titleLarge,
                        color = MaterialTheme.colorScheme.onSecondaryContainer
                    )
                    Button(
                        onClick = { viewModel.refreshRequests() },
                        modifier = Modifier.height(36.dp)
                    ) {
                        Text("Refresh")
                    }
                }
                
                if (uiState.pendingRequests.isNotEmpty()) {
                    LazyColumn(
                        verticalArrangement = Arrangement.spacedBy(8.dp),
                        modifier = Modifier.height(200.dp)
                    ) {
                        items(uiState.pendingRequests.size) { index ->
                            RequestCard(uiState.pendingRequests[index], viewModel)
                        }
                    }
                } else {
                    Text(
                        text = "No pending requests",
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        }
        
        // Server Mode Toggle
        Card(
            modifier = Modifier.fillMaxWidth(),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = "Server Mode",
                    style = MaterialTheme.typography.titleMedium
                )
                Switch(
                    checked = uiState.isServerMode,
                    onCheckedChange = {
                        if (it) viewModel.startServerMode() else viewModel.stopServerMode()
                    }
                )
            }
        }
        
        // Action Buttons
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Button(
                onClick = { viewModel.submitTestTask() },
                modifier = Modifier.weight(1f),
                enabled = uiState.isConnected
            ) {
                Text("Submit Test Task")
            }
        }
    }
}

@Composable
fun ResourceBar(label: String, value: Float) {
    Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(
                text = label,
                style = MaterialTheme.typography.bodyMedium
            )
            Text(
                text = "${String.format("%.1f", value)}%",
                style = MaterialTheme.typography.bodyMedium
            )
        }
        LinearProgressIndicator(
            progress = value / 100f,
            modifier = Modifier.fillMaxWidth()
        )
    }
}


@Composable
fun RequestCard(request: ConnectionRequest, viewModel: MainViewModel) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        elevation = CardDefaults.cardElevation(defaultElevation = 2.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant
        )
    ) {
        Column(
            modifier = Modifier.padding(12.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Text(
                text = request.deviceName,
                style = MaterialTheme.typography.titleSmall
            )
            Text(
                text = "${request.deviceType} @ ${request.deviceAddress}",
                style = MaterialTheme.typography.bodySmall
            )
            Text(
                text = "Wants to recruit you",
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.primary
            )
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Button(
                    onClick = { viewModel.acceptRequest(request) },
                    modifier = Modifier.weight(1f),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.primary
                    )
                ) {
                    Icon(Icons.Default.Check, contentDescription = "Accept")
                    Spacer(modifier = Modifier.width(4.dp))
                    Text("Accept")
                }
                Button(
                    onClick = { viewModel.rejectRequest(request) },
                    modifier = Modifier.weight(1f),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = MaterialTheme.colorScheme.error
                    )
                ) {
                    Icon(Icons.Default.Close, contentDescription = "Reject")
                    Spacer(modifier = Modifier.width(4.dp))
                    Text("Reject")
                }
            }
        }
    }
}

