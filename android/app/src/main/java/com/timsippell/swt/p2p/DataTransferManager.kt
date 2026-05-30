package com.timsippell.swt.p2p

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.coroutines.withTimeoutOrNull
import java.io.DataInputStream
import java.io.DataOutputStream
import java.net.InetAddress
import java.net.InetSocketAddress
import java.net.ServerSocket
import java.net.Socket
import java.security.MessageDigest

sealed class TransferResult {
    data class Success(val json: String) : TransferResult()
    data class Failure(val reason: String) : TransferResult()
}

class DataTransferManager {

    companion object {
        const val PORT = 9578
        private const val CHUNK_SIZE = 8192
        private const val MAX_SIZE = 50L * 1024 * 1024
    }

    suspend fun startServer(
        expectedSessionId: String,
        onProgress: (Float) -> Unit
    ): TransferResult = withContext(Dispatchers.IO) {
        var serverSocket: ServerSocket? = null
        try {
            serverSocket = ServerSocket(PORT)
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

            val dataSize = input.readLong()
            if (dataSize <= 0 || dataSize > MAX_SIZE) {
                socket.close()
                return@withContext TransferResult.Failure("Invalid data size: $dataSize")
            }

            val expectedHash = ByteArray(32)
            input.readFully(expectedHash)

            val data = ByteArray(dataSize.toInt())
            var bytesRead = 0
            while (bytesRead < dataSize) {
                val toRead = minOf(CHUNK_SIZE, dataSize.toInt() - bytesRead)
                val read = input.read(data, bytesRead, toRead)
                if (read == -1) break
                bytesRead += read
                onProgress(bytesRead.toFloat() / dataSize)
            }

            socket.close()

            val actualHash = MessageDigest.getInstance("SHA-256").digest(data)
            if (!actualHash.contentEquals(expectedHash)) {
                return@withContext TransferResult.Failure("Data corrupted (hash mismatch)")
            }

            TransferResult.Success(String(data, Charsets.UTF_8))
        } catch (e: Exception) {
            TransferResult.Failure("Transfer failed: ${e.message}")
        } finally {
            try { serverSocket?.close() } catch (_: Exception) {}
        }
    }

    suspend fun sendData(
        hostAddress: InetAddress,
        sessionId: String,
        json: String,
        onProgress: (Float) -> Unit
    ): TransferResult = withContext(Dispatchers.IO) {
        try {
            val data = json.toByteArray(Charsets.UTF_8)
            val hash = MessageDigest.getInstance("SHA-256").digest(data)

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
                output.writeLong(data.size.toLong())
                output.write(hash)

                var bytesSent = 0
                while (bytesSent < data.size) {
                    val toWrite = minOf(CHUNK_SIZE, data.size - bytesSent)
                    output.write(data, bytesSent, toWrite)
                    bytesSent += toWrite
                    onProgress(bytesSent.toFloat() / data.size)
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
}
