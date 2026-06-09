package com.timsippell.owt.p2p

import com.timsippell.owt.bridge.OwtBridge

sealed interface ShareState {
    data object Idle : ShareState
    data object SelectingWorkouts : ShareState
    data object WaitingForTap : ShareState
    data class NfcConnected(val sessionId: String) : ShareState
    data object ConnectingWifiDirect : ShareState
    data class Transferring(val progress: Float) : ShareState
    data class ReviewingImport(val summary: OwtBridge.ImportSummary, val json: String) : ShareState
    data class Complete(val message: String) : ShareState
    data class Error(val message: String) : ShareState
}

enum class ShareRole { SEND, RECEIVE }
