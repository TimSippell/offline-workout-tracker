package com.timsippell.owt.p2p

import android.annotation.SuppressLint
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.net.wifi.p2p.WifiP2pConfig
import android.net.wifi.p2p.WifiP2pInfo
import android.net.wifi.p2p.WifiP2pManager
import android.os.Build
import kotlinx.coroutines.CompletableDeferred
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlinx.coroutines.withTimeoutOrNull
import kotlin.coroutines.resume

@SuppressLint("MissingPermission")
class WifiDirectManager(private val context: Context) {

    private var manager: WifiP2pManager? = null
    private var channel: WifiP2pManager.Channel? = null
    private var receiver: BroadcastReceiver? = null
    private val connectionDeferred = CompletableDeferred<WifiP2pInfo>()

    fun initialize() {
        manager = context.getSystemService(Context.WIFI_P2P_SERVICE) as? WifiP2pManager
        channel = manager?.initialize(context, context.mainLooper, null)
        registerReceiver()
    }

    var lastFailureReason: Int = -1
        private set

    suspend fun createGroup(): Boolean {
        val mgr = manager ?: return false
        val ch = channel ?: return false

        // Remove any leftover group first
        suspendCancellableCoroutine { cont ->
            mgr.removeGroup(ch, object : WifiP2pManager.ActionListener {
                override fun onSuccess() { cont.resume(Unit) }
                override fun onFailure(reason: Int) { cont.resume(Unit) }
            })
        }

        return suspendCancellableCoroutine { cont ->
            mgr.createGroup(ch, object : WifiP2pManager.ActionListener {
                override fun onSuccess() { cont.resume(true) }
                override fun onFailure(reason: Int) {
                    lastFailureReason = reason
                    android.util.Log.e("WifiDirectManager", "createGroup failed: reason=$reason")
                    cont.resume(false)
                }
            })
        }
    }

    suspend fun discoverAndConnect(timeoutMs: Long = 30_000): WifiP2pInfo? {
        val mgr = manager ?: return null
        val ch = channel ?: return null

        val discovered = suspendCancellableCoroutine { cont ->
            mgr.discoverPeers(ch, object : WifiP2pManager.ActionListener {
                override fun onSuccess() { cont.resume(true) }
                override fun onFailure(reason: Int) { cont.resume(false) }
            })
        }
        if (!discovered) return null

        return withTimeoutOrNull(timeoutMs) {
            waitForPeersAndConnect(mgr, ch)
            connectionDeferred.await()
        }
    }

    suspend fun waitForConnection(timeoutMs: Long = 30_000): WifiP2pInfo? {
        return withTimeoutOrNull(timeoutMs) {
            connectionDeferred.await()
        }
    }

    fun teardown() {
        val mgr = manager ?: return
        val ch = channel ?: return
        try {
            mgr.removeGroup(ch, null)
            mgr.cancelConnect(ch, null)
        } catch (_: Exception) {}
        try {
            context.unregisterReceiver(receiver)
        } catch (_: Exception) {}
        receiver = null
    }

    private suspend fun waitForPeersAndConnect(mgr: WifiP2pManager, ch: WifiP2pManager.Channel) {
        suspendCancellableCoroutine { cont ->
            mgr.requestPeers(ch) { peerList ->
                val device = peerList.deviceList.firstOrNull()
                if (device != null) {
                    val config = WifiP2pConfig().apply {
                        deviceAddress = device.deviceAddress
                        groupOwnerIntent = 0
                    }
                    mgr.connect(ch, config, object : WifiP2pManager.ActionListener {
                        override fun onSuccess() { cont.resume(Unit) }
                        override fun onFailure(reason: Int) { cont.resume(Unit) }
                    })
                } else {
                    cont.resume(Unit)
                }
            }
        }
    }

    private fun registerReceiver() {
        val filter = IntentFilter().apply {
            addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION)
            addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION)
            addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION)
            addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION)
        }

        receiver = object : BroadcastReceiver() {
            @SuppressLint("MissingPermission")
            override fun onReceive(ctx: Context, intent: Intent) {
                when (intent.action) {
                    WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION -> {
                        manager?.requestPeers(channel) { peerList ->
                            val device = peerList.deviceList.firstOrNull()
                            if (device != null && !connectionDeferred.isCompleted) {
                                val config = WifiP2pConfig().apply {
                                    deviceAddress = device.deviceAddress
                                    groupOwnerIntent = 0
                                }
                                manager?.connect(channel, config, null)
                            }
                        }
                    }
                    WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION -> {
                        manager?.requestConnectionInfo(channel) { info ->
                            if (info?.groupFormed == true && !connectionDeferred.isCompleted) {
                                connectionDeferred.complete(info)
                            }
                        }
                    }
                }
            }
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            context.registerReceiver(receiver, filter, Context.RECEIVER_NOT_EXPORTED)
        } else {
            context.registerReceiver(receiver, filter)
        }
    }
}
