package utils

import org.junit.Assert
import org.junit.Test
import utils.IP.intIPv6toString
import utils.IP.stringIPv4ToInt

class IPTest {

    @Test
    fun ipConverterTest() {
        val expected = "127.0.0.1"
        val intIPv4 = stringIPv4ToInt(expected)
        println(intIPv4.toString(2))
        val result = intIPv6toString(intIPv4)
        Assert.assertEquals(expected, result)
    }

}
