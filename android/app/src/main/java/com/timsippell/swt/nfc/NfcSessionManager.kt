package com.timsippell.swt.nfc

import android.app.Activity
import android.content.ComponentName
import android.content.pm.PackageManager
import android.nfc.NfcAdapter
import android.nfc.Tag
import android.nfc.tech.IsoDep
import android.os.Bundle
import androidx.lifecycle.DefaultLifecycleObserver
import androidx.lifecycle.LifecycleOwner

class NfcSessionManager : DefaultLifecycleObserver {

    private var activity: Activity? = null
    private var readerCallback: NfcAdapter.ReaderCallback? = null
    private var isReaderActive = false

    companion object {
        private val SELECT_AID_APDU = byteArrayOf(
            0x00.toByte(), 0xA4.toByte(), 0x04.toByte(), 0x00.toByte(),
            0x08.toByte(),
            0xF0.toByte(), 0x53.toByte(), 0x57.toByte(), 0x54.toByte(),
            0x54.toByte(), 0x52.toByte(), 0x4E.toByte(), 0x53.toByte(),
            0x00.toByte()
        )
        private val GET_DATA_APDU = byteArrayOf(
            0x00.toByte(), 0xCA.toByte(), 0x00.toByte(), 0x00.toByte()
        )

        fun isNfcAvailable(activity: Activity): Boolean {
            return NfcAdapter.getDefaultAdapter(activity) != null
        }

        fun isNfcEnabled(activity: Activity): Boolean {
            return NfcAdapter.getDefaultAdapter(activity)?.isEnabled == true
        }
    }

    fun startAsReceiver(activity: Activity, onSessionReceived: (String) -> Unit) {
        this.activity = activity
        val adapter = NfcAdapter.getDefaultAdapter(activity) ?: return

        readerCallback = NfcAdapter.ReaderCallback { tag ->
            readSessionFromTag(tag)?.let { sessionId ->
                activity.runOnUiThread { onSessionReceived(sessionId) }
            }
        }

        val options = Bundle().apply {
            putInt(NfcAdapter.EXTRA_READER_PRESENCE_CHECK_DELAY, 2000)
        }
        adapter.enableReaderMode(
            activity,
            readerCallback,
            NfcAdapter.FLAG_READER_NFC_A or NfcAdapter.FLAG_READER_NFC_B or NfcAdapter.FLAG_READER_SKIP_NDEF_CHECK,
            options
        )
        isReaderActive = true
    }

    fun startAsSender(activity: Activity, sessionId: String) {
        this.activity = activity
        WorkoutHceService.sessionId.set(sessionId)
        WorkoutHceService.tapDetected.set(false)

        val componentName = ComponentName(activity, WorkoutHceService::class.java)
        activity.packageManager.setComponentEnabledSetting(
            componentName,
            PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
            PackageManager.DONT_KILL_APP
        )
    }

    fun isTapDetected(): Boolean = WorkoutHceService.tapDetected.get()

    fun stop() {
        val act = activity ?: return

        if (isReaderActive) {
            NfcAdapter.getDefaultAdapter(act)?.disableReaderMode(act)
            isReaderActive = false
            readerCallback = null
        }

        val componentName = ComponentName(act, WorkoutHceService::class.java)
        act.packageManager.setComponentEnabledSetting(
            componentName,
            PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
            PackageManager.DONT_KILL_APP
        )
        WorkoutHceService.sessionId.set(null)
        WorkoutHceService.tapDetected.set(false)
    }

    override fun onPause(owner: LifecycleOwner) {
        val act = activity ?: return
        if (isReaderActive) {
            NfcAdapter.getDefaultAdapter(act)?.disableReaderMode(act)
        }
    }

    override fun onResume(owner: LifecycleOwner) {
        val act = activity ?: return
        if (isReaderActive && readerCallback != null) {
            val adapter = NfcAdapter.getDefaultAdapter(act) ?: return
            val options = Bundle().apply {
                putInt(NfcAdapter.EXTRA_READER_PRESENCE_CHECK_DELAY, 2000)
            }
            adapter.enableReaderMode(
                act,
                readerCallback,
                NfcAdapter.FLAG_READER_NFC_A or NfcAdapter.FLAG_READER_NFC_B or NfcAdapter.FLAG_READER_SKIP_NDEF_CHECK,
                options
            )
        }
    }

    private fun readSessionFromTag(tag: Tag): String? {
        val isoDep = IsoDep.get(tag) ?: return null
        return try {
            isoDep.connect()
            isoDep.timeout = 5000

            val selectResponse = isoDep.transceive(SELECT_AID_APDU)
            if (!isSuccess(selectResponse)) return null

            val dataResponse = isoDep.transceive(GET_DATA_APDU)
            if (!isSuccess(dataResponse)) return null

            String(dataResponse, 0, dataResponse.size - 2, Charsets.UTF_8)
        } catch (_: Exception) {
            null
        } finally {
            try { isoDep.close() } catch (_: Exception) {}
        }
    }

    private fun isSuccess(response: ByteArray): Boolean {
        return response.size >= 2
                && response[response.size - 2] == 0x90.toByte()
                && response[response.size - 1] == 0x00.toByte()
    }
}
