package com.timsippell.swt.p2p

import com.timsippell.swt.bridge.SwtBridge

sealed interface ShareState {
    data object Idle : ShareState
    data object SelectingWorkouts : ShareState
    data object WaitingForTap : ShareState
    data class NfcConnected(val sessionId: String) : ShareState
    data object ConnectingWifiDirect : ShareState
    data class Transferring(val progress: Float) : ShareState
    data class ReviewingImport(val summary: SwtBridge.ImportSummary, val json: String) : ShareState
    data class Complete(val message: String) : ShareState
    data class Error(val message: String) : ShareState
}

enum class ShareRole { SEND, RECEIVE }
