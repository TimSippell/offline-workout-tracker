package com.timsippell.owt.p2p

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.DataInputStream
import java.io.DataOutputStream
import java.net.InetAddress
import java.net.InetSocketAddress
import java.net.ServerSocket
import java.net.Socket
import java.security.SecureRandom
import javax.crypto.Cipher
import javax.crypto.spec.GCMParameterSpec
import javax.crypto.spec.SecretKeySpec

sealed class TransferResult {
    data class Success(val json: String) : TransferResult()
    data class Failure(val reason: String) : TransferResult()
}

class DataTransferManager {

    companion object {
        const val PORT = 9578
        private const val CHUNK_SIZE = 8192
        private const val MAX_SIZE = 50L * 1024 * 1024
        private const val GCM_IV_SIZE = 12
        private const val GCM_TAG_BITS = 128
    }

    suspend fun startServer(
        bindAddress: InetAddress,
        expectedSessionId: String,
        sharedSecret: ByteArray,
        onProgress: (Float) -> Unit
    ): TransferResult = withContext(Dispatchers.IO) {
        var serverSocket: ServerSocket? = null
        try {
            serverSocket = ServerSocket(PORT, 1, bindAddress)
            serverSocket.soTimeout = 30_000

            val socket = serverSocket.accept()
            socket.soTimeout = 15_000

            val input = DataInputStream(socket.getInputStream())

            val sessionBytes = ByteArray(36)
            input.readFully(sessionBytes)
            val receivedSession = String(sessionBytes, Charsets.UTF_8)
            if (receivedSession != expectedSessionId) {
                socket.close()
                return@withContext TransferResult.Failure("Session mismatch")
            }

            val iv = ByteArray(GCM_IV_SIZE)
            input.readFully(iv)

            val ciphertextSize = input.readLong()
            if (ciphertextSize <= 0 || ciphertextSize > MAX_SIZE) {
                socket.close()
                return@withContext TransferResult.Failure("Invalid data size: $ciphertextSize")
            }
            if (ciphertextSize > Int.MAX_VALUE) {
                socket.close()
                return@withContext TransferResult.Failure("Data too large")
            }

            val ciphertext = ByteArray(ciphertextSize.toInt())
            var bytesRead = 0
            while (bytesRead < ciphertextSize.toInt()) {
                val toRead = minOf(CHUNK_SIZE, ciphertextSize.toInt() - bytesRead)
                val read = input.read(ciphertext, bytesRead, toRead)
                if (read == -1) break
                bytesRead += read
                onProgress(bytesRead.toFloat() / ciphertextSize)
            }

            socket.close()

            val plaintext = try {
                decrypt(ciphertext, sharedSecret, iv)
            } catch (e: Exception) {
                return@withContext TransferResult.Failure("Decryption failed — wrong key or corrupted data")
            }

            TransferResult.Success(String(plaintext, Charsets.UTF_8))
        } catch (e: Exception) {
            TransferResult.Failure("Transfer failed: ${e.message}")
        } finally {
            try { serverSocket?.close() } catch (_: Exception) {}
        }
    }

    suspend fun sendData(
        hostAddress: InetAddress,
        sessionId: String,
        sharedSecret: ByteArray,
        json: String,
        onProgress: (Float) -> Unit
    ): TransferResult = withContext(Dispatchers.IO) {
        try {
            val plaintext = json.toByteArray(Charsets.UTF_8)
            val iv = ByteArray(GCM_IV_SIZE).also { SecureRandom().nextBytes(it) }
            val ciphertext = encrypt(plaintext, sharedSecret, iv)

            var lastSocket: Socket? = null
            for (attempt in 1..3) {
                try {
                    val s = Socket()
                    s.connect(InetSocketAddress(hostAddress, PORT), 10_000)
                    lastSocket = s
                    break
                } catch (e: Exception) {
                    if (attempt == 3) throw e
                    Thread.sleep(2000)
                }
            }
            val socket = lastSocket ?: return@withContext TransferResult.Failure("Could not connect")

            try {
                val output = DataOutputStream(socket.outputStream)

                output.write(sessionId.toByteArray(Charsets.UTF_8))
                output.write(iv)
                output.writeLong(ciphertext.size.toLong())

                var bytesSent = 0
                while (bytesSent < ciphertext.size) {
                    val toWrite = minOf(CHUNK_SIZE, ciphertext.size - bytesSent)
                    output.write(ciphertext, bytesSent, toWrite)
                    bytesSent += toWrite
                    onProgress(bytesSent.toFloat() / ciphertext.size)
                }
                output.flush()
                TransferResult.Success(json)
            } finally {
                try { socket.close() } catch (_: Exception) {}
            }
        } catch (e: Exception) {
            TransferResult.Failure("Send failed: ${e.message}")
        }
    }

    private fun encrypt(data: ByteArray, key: ByteArray, iv: ByteArray): ByteArray {
        val cipher = Cipher.getInstance("AES/GCM/NoPadding")
        cipher.init(Cipher.ENCRYPT_MODE, SecretKeySpec(key, "AES"), GCMParameterSpec(GCM_TAG_BITS, iv))
        return cipher.doFinal(data)
    }

    private fun decrypt(ciphertext: ByteArray, key: ByteArray, iv: ByteArray): ByteArray {
        val cipher = Cipher.getInstance("AES/GCM/NoPadding")
        cipher.init(Cipher.DECRYPT_MODE, SecretKeySpec(key, "AES"), GCMParameterSpec(GCM_TAG_BITS, iv))
        return cipher.doFinal(ciphertext)
    }
}
