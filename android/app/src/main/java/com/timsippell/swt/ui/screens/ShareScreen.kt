package com.timsippell.swt.ui.screens

import android.Manifest
import android.content.Context
import android.content.Intent
import android.location.LocationManager
import android.os.Build
import android.provider.Settings
import androidx.activity.ComponentActivity
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.filled.CheckCircle
import androidx.compose.material.icons.filled.Nfc
import androidx.compose.material.icons.filled.PhoneAndroid
import androidx.compose.material.icons.filled.Warning
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.timsippell.swt.bridge.SwtBridge
import com.timsippell.swt.nfc.NfcSessionManager
import com.timsippell.swt.p2p.ShareSession
import com.timsippell.swt.p2p.ShareState

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ShareScreen(onNavigateBack: () -> Unit) {
    val activity = LocalContext.current as ComponentActivity
    val shareSession = remember { ShareSession(activity) }
    val state by shareSession.state.collectAsState()

    var hasPermissions by remember { mutableStateOf(false) }

    val permissionLauncher = rememberLauncherForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { results ->
        hasPermissions = results.values.all { it }
    }

    LaunchedEffect(Unit) {
        val permissions = buildList {
            add(Manifest.permission.ACCESS_FINE_LOCATION)
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
                add(Manifest.permission.NEARBY_WIFI_DEVICES)
            }
        }
        permissionLauncher.launch(permissions.toTypedArray())
    }

    DisposableEffect(Unit) {
        onDispose { shareSession.destroy() }
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Share Workouts") },
                navigationIcon = {
                    IconButton(onClick = {
                        shareSession.cancel()
                        onNavigateBack()
                    }) {
                        Icon(Icons.AutoMirrored.Filled.ArrowBack, contentDescription = "Back")
                    }
                }
            )
        }
    ) { padding ->
        Box(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(24.dp),
            contentAlignment = Alignment.Center
        ) {
            when (val s = state) {
                ShareState.Idle -> IdleContent(
                    activity = activity,
                    onSend = { shareSession.promptSend() },
                    onReceive = { shareSession.startReceiving() }
                )
                ShareState.SelectingWorkouts -> SelectWorkoutsContent(
                    onConfirm = { selectedIds -> shareSession.startSending(selectedIds) },
                    onCancel = { shareSession.cancel() }
                )
                ShareState.WaitingForTap -> WaitingForTapContent(
                    onCancel = { shareSession.cancel() }
                )
                is ShareState.NfcConnected -> NfcConnectedContent()
                ShareState.ConnectingWifiDirect -> ConnectingContent()
                is ShareState.Transferring -> TransferringContent(s.progress)
                is ShareState.ReviewingImport -> ReviewingImportContent(
                    summary = s.summary,
                    onConfirm = { shareSession.confirmImport(s.json) },
                    onCancel = { shareSession.cancel() }
                )
                is ShareState.Complete -> CompleteContent(
                    message = s.message,
                    onDone = {
                        shareSession.reset()
                    }
                )
                is ShareState.Error -> ErrorContent(
                    message = s.message,
                    onRetry = { shareSession.reset() },
                    onBack = {
                        shareSession.cancel()
                        onNavigateBack()
                    }
                )
            }
        }
    }
}

@Composable
private fun IdleContent(
    activity: ComponentActivity,
    onSend: () -> Unit,
    onReceive: () -> Unit
) {
    val nfcAvailable = remember { NfcSessionManager.isNfcAvailable(activity) }
    val nfcEnabled = remember { NfcSessionManager.isNfcEnabled(activity) }
    val locationManager = remember {
        activity.getSystemService(Context.LOCATION_SERVICE) as LocationManager
    }
    val locationEnabled = remember {
        locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER) ||
            locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)
    }

    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(24.dp)
    ) {
        Icon(
            Icons.Default.Nfc,
            contentDescription = null,
            modifier = Modifier.size(72.dp),
            tint = MaterialTheme.colorScheme.primary
        )

        if (!nfcAvailable) {
            Text(
                "This device does not support NFC",
                style = MaterialTheme.typography.bodyLarge,
                textAlign = TextAlign.Center
            )
            Text(
                "Use Export/Import from Settings to share data via file instead.",
                style = MaterialTheme.typography.bodyMedium,
                textAlign = TextAlign.Center,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        } else if (!nfcEnabled) {
            Text(
                "NFC is turned off",
                style = MaterialTheme.typography.bodyLarge,
                textAlign = TextAlign.Center
            )
            OutlinedButton(onClick = {
                activity.startActivity(Intent(Settings.ACTION_NFC_SETTINGS))
            }) {
                Text("Open NFC Settings")
            }
        } else if (!locationEnabled) {
            Text(
                "Location services are required for Wi-Fi Direct",
                style = MaterialTheme.typography.bodyLarge,
                textAlign = TextAlign.Center
            )
            Text(
                "Android requires Location to be enabled for device-to-device sharing.",
                style = MaterialTheme.typography.bodyMedium,
                textAlign = TextAlign.Center,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            OutlinedButton(onClick = {
                activity.startActivity(Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS))
            }) {
                Text("Open Location Settings")
            }
        } else {
            Text(
                "Share workout data with another device",
                style = MaterialTheme.typography.bodyLarge,
                textAlign = TextAlign.Center
            )

            Button(
                onClick = onSend,
                modifier = Modifier.fillMaxWidth()
            ) {
                Text("Send My Data")
            }

            OutlinedButton(
                onClick = onReceive,
                modifier = Modifier.fillMaxWidth()
            ) {
                Text("Receive Data")
            }

            Text(
                "Both devices need the app open.\nTap phones back-to-back to start.",
                style = MaterialTheme.typography.bodySmall,
                textAlign = TextAlign.Center,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
    }
}

@Composable
private fun SelectWorkoutsContent(
    onConfirm: (Set<Long>) -> Unit,
    onCancel: () -> Unit
) {
    val templates = remember { SwtBridge.listTemplates() }
    val selectedIds = remember { mutableStateMapOf<Long, Boolean>() }

    LaunchedEffect(templates) {
        templates.forEach { selectedIds[it.id] = true }
    }

    Column(
        modifier = Modifier.fillMaxSize(),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text("Select Templates to Send", style = MaterialTheme.typography.headlineSmall)

        if (templates.isEmpty()) {
            Text(
                "No templates to send",
                style = MaterialTheme.typography.bodyLarge,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        } else {
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                TextButton(onClick = { templates.forEach { selectedIds[it.id] = true } }) {
                    Text("Select All")
                }
                TextButton(onClick = { selectedIds.keys.forEach { selectedIds[it] = false } }) {
                    Text("Select None")
                }
            }

            LazyColumn(
                modifier = Modifier.weight(1f),
                verticalArrangement = Arrangement.spacedBy(2.dp)
            ) {
                items(templates, key = { it.id }) { template ->
                    val checked = selectedIds[template.id] == true
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clickable { selectedIds[template.id] = !checked }
                            .padding(vertical = 8.dp, horizontal = 4.dp),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.spacedBy(12.dp)
                    ) {
                        Checkbox(checked = checked, onCheckedChange = { selectedIds[template.id] = it })
                        Column(modifier = Modifier.weight(1f)) {
                            Text(
                                template.name,
                                style = MaterialTheme.typography.bodyLarge
                            )
                            Text(
                                "${template.setCount} sets",
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    }
                }
            }
        }

        val count = selectedIds.count { it.value }
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(12.dp, Alignment.End)
        ) {
            OutlinedButton(onClick = onCancel) { Text("Cancel") }
            Button(
                onClick = { onConfirm(selectedIds.filter { it.value }.keys) },
                enabled = count > 0
            ) {
                Text("Send $count template${if (count != 1) "s" else ""}")
            }
        }
    }
}

@Composable
private fun WaitingForTapContent(onCancel: () -> Unit) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(24.dp)
    ) {
        Icon(
            Icons.Default.PhoneAndroid,
            contentDescription = null,
            modifier = Modifier.size(72.dp),
            tint = MaterialTheme.colorScheme.primary
        )
        Text(
            "Ready",
            style = MaterialTheme.typography.headlineMedium
        )
        Text(
            "Hold phones back-to-back",
            style = MaterialTheme.typography.bodyLarge,
            textAlign = TextAlign.Center
        )
        CircularProgressIndicator()
        OutlinedButton(onClick = onCancel) {
            Text("Cancel")
        }
    }
}

@Composable
private fun NfcConnectedContent() {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Icon(
            Icons.Default.CheckCircle,
            contentDescription = null,
            modifier = Modifier.size(48.dp),
            tint = MaterialTheme.colorScheme.primary
        )
        Text("NFC connected", style = MaterialTheme.typography.bodyLarge)
        CircularProgressIndicator()
    }
}

@Composable
private fun ConnectingContent() {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        CircularProgressIndicator()
        Text("Connecting via Wi-Fi Direct...", style = MaterialTheme.typography.bodyLarge)
    }
}

@Composable
private fun TransferringContent(progress: Float) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Text("Transferring...", style = MaterialTheme.typography.bodyLarge)
        LinearProgressIndicator(
            progress = { progress },
            modifier = Modifier.fillMaxWidth()
        )
        Text("${(progress * 100).toInt()}%", style = MaterialTheme.typography.bodyMedium)
    }
}

@Composable
private fun ReviewingImportContent(
    summary: SwtBridge.ImportSummary,
    onConfirm: () -> Unit,
    onCancel: () -> Unit
) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Text("Data Received", style = MaterialTheme.typography.headlineMedium)
        Card(modifier = Modifier.fillMaxWidth()) {
            Column(modifier = Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(4.dp)) {
                Text("${summary.newExercises} new exercises", style = MaterialTheme.typography.bodyMedium)
                if (summary.existingExercises > 0) {
                    Text(
                        "${summary.existingExercises} already exist",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                Text("${summary.workouts} workouts with ${summary.workoutSets} sets", style = MaterialTheme.typography.bodyMedium)
                Text("${summary.templates} templates with ${summary.templateSets} sets", style = MaterialTheme.typography.bodyMedium)
            }
        }
        Text(
            "Existing data will not be modified.",
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            OutlinedButton(onClick = onCancel) { Text("Cancel") }
            Button(onClick = onConfirm) { Text("Import") }
        }
    }
}

@Composable
private fun CompleteContent(message: String, onDone: () -> Unit) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Icon(
            Icons.Default.CheckCircle,
            contentDescription = null,
            modifier = Modifier.size(72.dp),
            tint = MaterialTheme.colorScheme.primary
        )
        Text(message, style = MaterialTheme.typography.bodyLarge, textAlign = TextAlign.Center)
        Button(onClick = onDone) { Text("Done") }
    }
}

@Composable
private fun ErrorContent(message: String, onRetry: () -> Unit, onBack: () -> Unit) {
    Column(
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Icon(
            Icons.Default.Warning,
            contentDescription = null,
            modifier = Modifier.size(72.dp),
            tint = MaterialTheme.colorScheme.error
        )
        Text(message, style = MaterialTheme.typography.bodyLarge, textAlign = TextAlign.Center)
        Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            OutlinedButton(onClick = onBack) { Text("Back") }
            Button(onClick = onRetry) { Text("Retry") }
        }
    }
}
