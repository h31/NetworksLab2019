package extensions

import org.junit.Assert.assertEquals
import org.junit.Test
import java.nio.ByteBuffer

class ByteBufferExtensionsTest {

    @Test
    fun consume() {
        val buffer = ByteBuffer.wrap("Hello, World!".toByteArray())
        val consumedHello = String(buffer.consume(5))
        assertEquals("Hello", consumedHello)
        val consumedCommaAndSpace = String(buffer.consume(2))
        assertEquals(", ", consumedCommaAndSpace)
        val consumedWorld = String(buffer.consume(6))
        assertEquals("World!", consumedWorld)
    }

    @Test(expected = IllegalArgumentException::class)
    fun consumeWithException() {
        val buffer = ByteBuffer.wrap("Hello, World!".toByteArray())
        buffer.consume(200)
    }

    @Test
    fun consumeShort() {
        val buffer = ByteBuffer.wrap(byteArrayOf(0b00000000, 0b00001010))
        val short = buffer.consumeShort()
        assertEquals(10, short.toInt())
    }

    @Test
    fun consumeInt() {
        val buffer = ByteBuffer.wrap(byteArrayOf(0b00000000, 0b00000000, 0b00000000, 0b00001010))
        val int = buffer.consumeInt()
        assertEquals(10, int)
    }

}
