package com.timsippell.swt.p2p

import androidx.activity.ComponentActivity
import androidx.lifecycle.DefaultLifecycleObserver
import androidx.lifecycle.LifecycleOwner
import com.timsippell.swt.bridge.SwtBridge
import com.timsippell.swt.nfc.NfcSessionManager
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import org.json.JSONArray
import org.json.JSONObject
import java.util.UUID

class ShareSession(private val activity: ComponentActivity) : DefaultLifecycleObserver {

    private val _state = MutableStateFlow<ShareState>(ShareState.Idle)
    val state: StateFlow<ShareState> = _state.asStateFlow()

    private val nfcManager = NfcSessionManager()
    private var wifiManager: WifiDirectManager? = null
    private val transferManager = DataTransferManager()
    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Main)
    private var currentSessionId: String? = null
    private var currentRole: ShareRole? = null

    init {
        activity.lifecycle.addObserver(nfcManager)
        activity.lifecycle.addObserver(this)
    }

    fun promptSend() {
        _state.value = ShareState.SelectingWorkouts
    }

    fun startSending(selectedTemplateIds: Set<Long>) {
        currentRole = ShareRole.SEND
        val sessionId = UUID.randomUUID().toString()
        currentSessionId = sessionId
        _state.value = ShareState.WaitingForTap

        nfcManager.startAsSender(activity, sessionId)

        scope.launch {
            val tapDetected = withTimeoutOrNull(120_000L) {
                while (!nfcManager.isTapDetected()) {
                    delay(200)
                }
                true
            }

            if (tapDetected != true) {
                _state.value = ShareState.Error("NFC tap timed out")
                cleanup()
                return@launch
            }

            _state.value = ShareState.NfcConnected(sessionId)
            delay(500)

            _state.value = ShareState.ConnectingWifiDirect
            wifiManager = WifiDirectManager(activity).also { it.initialize() }

            val connectionInfo = wifiManager?.discoverAndConnect()
            if (connectionInfo == null) {
                _state.value = ShareState.Error("Wi-Fi Direct connection failed")
                cleanup()
                return@launch
            }

            val groupOwnerAddress = connectionInfo.groupOwnerAddress
            if (groupOwnerAddress == null) {
                _state.value = ShareState.Error("Could not determine receiver address")
                cleanup()
                return@launch
            }

            _state.value = ShareState.Transferring(0f)
            val json = filterExportJson(SwtBridge.exportToJson(), selectedTemplateIds)
            val result = transferManager.sendData(groupOwnerAddress, sessionId, json) { progress ->
                _state.value = ShareState.Transferring(progress)
            }

            when (result) {
                is TransferResult.Success -> _state.value = ShareState.Complete("Data sent successfully")
                is TransferResult.Failure -> _state.value = ShareState.Error(result.reason)
            }
            cleanup()
        }
    }

    private fun filterExportJson(fullJson: String, selectedTemplateIds: Set<Long>): String {
        val root = JSONObject(fullJson)
        val templates = root.optJSONArray("templates") ?: return fullJson
        val exercises = root.optJSONArray("exercises")

        val filteredTemplates = JSONArray()
        val referencedExerciseIds = mutableSetOf<Long>()

        for (i in 0 until templates.length()) {
            val template = templates.getJSONObject(i)
            if (selectedTemplateIds.contains(template.getLong("id"))) {
                filteredTemplates.put(template)
                val sets = template.optJSONArray("sets")
                if (sets != null) {
                    for (j in 0 until sets.length()) {
                        referencedExerciseIds.add(sets.getJSONObject(j).getLong("exerciseId"))
                    }
                }
            }
        }

        val result = JSONObject()

        if (exercises != null) {
            val filteredExercises = JSONArray()
            for (i in 0 until exercises.length()) {
                val exercise = exercises.getJSONObject(i)
                if (referencedExerciseIds.contains(exercise.getLong("id"))) {
                    filteredExercises.put(exercise)
                }
            }
            result.put("exercises", filteredExercises)
        }

        result.put("templates", filteredTemplates)
        return result.toString()
    }

    fun startReceiving() {
        currentRole = ShareRole.RECEIVE
        _state.value = ShareState.WaitingForTap

        wifiManager = WifiDirectManager(activity).also { it.initialize() }

        scope.launch {
            val groupCreated = wifiManager?.createGroup() == true
            if (!groupCreated) {
                val reason = wifiManager?.lastFailureReason ?: -1
                val hint = when (reason) {
                    0 -> "Internal error — check Wi-Fi is on"
                    1 -> "Wi-Fi Direct not supported on this device"
                    2 -> "Wi-Fi Direct is busy — try again"
                    else -> "Check Wi-Fi and Location are enabled"
                }
                _state.value = ShareState.Error("Could not create Wi-Fi Direct group (code $reason). $hint")
                cleanup()
                return@launch
            }

            var receivedSessionId: String? = null
            nfcManager.startAsReceiver(activity) { sessionId ->
                receivedSessionId = sessionId
                currentSessionId = sessionId
            }

            val sessionReceived = withTimeoutOrNull(120_000L) {
                while (receivedSessionId == null) {
                    delay(200)
                }
                true
            }

            if (sessionReceived != true) {
                _state.value = ShareState.Error("NFC tap timed out")
                cleanup()
                return@launch
            }

            val sessionId = receivedSessionId!!
            _state.value = ShareState.NfcConnected(sessionId)
            delay(500)

            _state.value = ShareState.ConnectingWifiDirect
            val connectionInfo = wifiManager?.waitForConnection()
            if (connectionInfo == null) {
                _state.value = ShareState.Error("Wi-Fi Direct connection failed")
                cleanup()
                return@launch
            }

            _state.value = ShareState.Transferring(0f)
            val result = transferManager.startServer(sessionId) { progress ->
                _state.value = ShareState.Transferring(progress)
            }

            when (result) {
                is TransferResult.Success -> {
                    val summary = SwtBridge.previewImport(result.json)
                    if (summary != null) {
                        _state.value = ShareState.ReviewingImport(summary, result.json)
                    } else {
                        _state.value = ShareState.Error("Could not parse received data")
                    }
                }
                is TransferResult.Failure -> _state.value = ShareState.Error(result.reason)
            }
            cleanupConnections()
        }
    }

    fun confirmImport(json: String) {
        try {
            val result = SwtBridge.importFromJson(json)
            _state.value = ShareState.Complete(result)
        } catch (e: Exception) {
            _state.value = ShareState.Error("Import failed: ${e.message}")
        }
    }

    fun cancel() {
        cleanup()
        _state.value = ShareState.Idle
    }

    fun reset() {
        _state.value = ShareState.Idle
    }

    fun destroy() {
        cleanup()
        scope.cancel()
        activity.lifecycle.removeObserver(nfcManager)
        activity.lifecycle.removeObserver(this)
    }

    private fun cleanup() {
        cleanupConnections()
        nfcManager.stop()
    }

    private fun cleanupConnections() {
        wifiManager?.teardown()
        wifiManager = null
        currentSessionId = null
        currentRole = null
    }
}
