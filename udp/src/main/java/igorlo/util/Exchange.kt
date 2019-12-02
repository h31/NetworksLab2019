package igorlo.util

import org.apache.log4j.Logger
import java.net.Socket
import java.nio.ByteBuffer
import java.nio.ByteOrder

object Exchange {

    private val logger = Logger.getLogger(Exchange::class.java)

    fun readMessage(socket: Socket): String {
        logger.info("Ожидаю длину сообщения")
        var readArray = ByteArray(Int.SIZE_BYTES)
        socket.getInputStream().read(readArray)
        val length = ByteBuffer.wrap(readArray).order(ByteOrder.LITTLE_ENDIAN).getInt(0)
        logger.info("Длина сообщения $length")
        logger.info("Ожидаю сообщение")
        readArray = ByteArray(length)
        socket.getInputStream().read(readArray)
        logger.info("Сообщение прочитано")
        return String(ByteBuffer.wrap(readArray).order(ByteOrder.LITTLE_ENDIAN).array(), Charsets.UTF_8)
    }

    fun sendMessage(socket: Socket, msg: String) {
        logger.info("Формирую из текста сообщения массив байт")
        val writeArray = ByteBuffer.wrap(msg.toByteArray(Charsets.UTF_8)).order(ByteOrder.LITTLE_ENDIAN).array()
        val byteArray = ByteBuffer.allocate(Int.SIZE_BYTES).order(ByteOrder.LITTLE_ENDIAN).putInt(msg.length).array()
        logger.info("Отправляю длину сообщения")
        socket.getOutputStream().write(byteArray)
        logger.info("Отправляю сообщение")
        socket.getOutputStream().write(writeArray)
    }
}