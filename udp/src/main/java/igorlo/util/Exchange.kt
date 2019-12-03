package igorlo.util

import org.apache.log4j.Level
import org.apache.log4j.Logger
import java.net.Socket
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.util.*

object Exchange {

    private val logger = Logger.getLogger(Exchange::class.java)

    fun setLoggingLevel(level: Level) {
        logger.level = level
    }

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
        val textArray = ByteBuffer.wrap(msg.toByteArray(Charsets.UTF_8)).order(ByteOrder.LITTLE_ENDIAN).array()
        logger.info("Массив текста: \n${textArray.map { e -> Integer.toBinaryString(e.toInt()).padStart(8, '0').take(8) }}")
        logger.info("Длина массива текста: ${textArray.size}")
        val lengthArray = ByteBuffer.allocate(Int.SIZE_BYTES).order(ByteOrder.LITTLE_ENDIAN).putInt(textArray.size).array()
        logger.info("Массив длины: \n${Arrays.toString(lengthArray)}")
        logger.info("Длина массива длины: ${lengthArray.size}")
        if (msg.length != textArray.size) {
            logger.info("Длина сообщения не равна кол-ву байт сообщения!")
        }
        logger.info("Отправляю длину сообщения")
        socket.getOutputStream().write(lengthArray)
        logger.info("Отправляю сообщение")
        socket.getOutputStream().write(textArray)
    }
}