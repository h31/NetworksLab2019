package extensions

import org.junit.Assert.assertEquals
import org.junit.Test

class ShortExtensionsTest {

    @Test
    fun highestByte() {
        val short: Short = 0b01100000_00001111
        assertEquals(0b01100000, short.highestByte().toInt())
    }

    @Test
    fun lowestByte() {
        val short: Short = 0b01100000_00001111
        assertEquals(0b00001111, short.lowestByte().toInt())
    }

}
