package com.timsippell.swt.nfc

import android.nfc.cardemulation.HostApduService
import android.os.Bundle
import java.util.concurrent.atomic.AtomicReference

class WorkoutHceService : HostApduService() {

    companion object {
        val sessionId = AtomicReference<String>(null)
        val tapDetected = AtomicReference(false)

        private val SELECT_AID = byteArrayOf(
            0x00.toByte(), 0xA4.toByte(), 0x04.toByte(), 0x00.toByte(),
            0x08.toByte(),
            0xF0.toByte(), 0x53.toByte(), 0x57.toByte(), 0x54.toByte(),
            0x54.toByte(), 0x52.toByte(), 0x4E.toByte(), 0x53.toByte()
        )
        private val GET_DATA_HEADER = byteArrayOf(
            0x00.toByte(), 0xCA.toByte(), 0x00.toByte(), 0x00.toByte()
        )
        private val SW_OK = byteArrayOf(0x90.toByte(), 0x00.toByte())
        private val SW_NOT_FOUND = byteArrayOf(0x6A.toByte(), 0x82.toByte())
    }

    override fun processCommandApdu(commandApdu: ByteArray, extras: Bundle?): ByteArray {
        if (commandApdu.size >= SELECT_AID.size && isSelectAid(commandApdu)) {
            return byteArrayOf(0x01) + SW_OK
        }
        if (commandApdu.size >= 4 && commandApdu[0] == GET_DATA_HEADER[0]
            && commandApdu[1] == GET_DATA_HEADER[1]
            && commandApdu[2] == GET_DATA_HEADER[2]
            && commandApdu[3] == GET_DATA_HEADER[3]
        ) {
            val id = sessionId.get() ?: return SW_NOT_FOUND
            tapDetected.set(true)
            return id.toByteArray(Charsets.UTF_8) + SW_OK
        }
        return SW_NOT_FOUND
    }

    override fun onDeactivated(reason: Int) {}

    private fun isSelectAid(apdu: ByteArray): Boolean {
        if (apdu[0] != 0x00.toByte() || apdu[1] != 0xA4.toByte()) return false
        if (apdu[2] != 0x04.toByte()) return false
        for (i in 5 until minOf(apdu.size, SELECT_AID.size)) {
            if (apdu[i] != SELECT_AID[i]) return false
        }
        return true
    }
}
