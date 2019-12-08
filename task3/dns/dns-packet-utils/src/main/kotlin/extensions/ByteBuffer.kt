package extensions

import java.nio.ByteBuffer

fun ByteBuffer.consume(bytes: Short) = consume(bytes.toInt())

fun ByteBuffer.consume(bytes: Int): ByteArray {
    check(remaining(), bytes)
    val startPosition = position()
    position(startPosition + bytes)
    return array().copyOfRange(startPosition, startPosition + bytes)
}

fun ByteBuffer.consumeByte(): Byte {
    check(remaining(), Byte.SIZE_BYTES)
    val startPosition = position()
    position(startPosition + Byte.SIZE_BYTES)
    return get(startPosition)
}

fun ByteBuffer.consumeShort(): Short {
    check(remaining(), Short.SIZE_BYTES)
    val startPosition = position()
    position(startPosition + Short.SIZE_BYTES)
    return getShort(startPosition)
}

fun ByteBuffer.consumeInt(): Int {
    check(remaining(), Int.SIZE_BYTES)
    val startPosition = position()
    position(startPosition + Int.SIZE_BYTES)
    return getInt(startPosition)
}

private fun check(remaining: Int, requires: Int) {
    require(remaining >= requires) {
        "Can't consume $requires bytes. There are only $remaining bytes in ByteBuffer"
    }
}
