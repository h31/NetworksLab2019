package extensions

import domain.model.enums.DNSMessageType
import domain.model.enums.DNSOpCode
import domain.model.enums.DNSRCode
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import utils.DNSPacketBuilder

class DNSFlagsTest {

    @Test
    fun build() {
        val dnsFlags1 = DNSPacketBuilder.buildFlags(0b1000_0000_0000_0000.toShort())
        assertEquals(DNSMessageType.QUERY, dnsFlags1.qr)
        assertEquals(DNSOpCode.STANDARD, dnsFlags1.opCode)
        assertFalse(dnsFlags1.aa)
        assertFalse(dnsFlags1.tc)
        assertFalse(dnsFlags1.rd)
        assertFalse(dnsFlags1.ra)
        assertEquals(DNSRCode.NO_ERROR, dnsFlags1.rCode)

        val dnsFlags2 = DNSPacketBuilder.buildFlags(0b0000_0111_1000_0011.toShort())
        assertEquals(DNSMessageType.ANSWER, dnsFlags2.qr)
        assertEquals(DNSOpCode.STANDARD, dnsFlags2.opCode)
        assertTrue(dnsFlags2.aa)
        assertTrue(dnsFlags2.tc)
        assertTrue(dnsFlags2.rd)
        assertTrue(dnsFlags2.ra)
        assertEquals(DNSRCode.NAME_ERROR, dnsFlags2.rCode)
    }

}
