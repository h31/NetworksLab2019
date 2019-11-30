package utils

import domain.model.DNSFlags
import domain.model.DNSHeader
import domain.model.DNSName
import domain.model.DNSPacket
import domain.model.DNSQuery
import domain.model.DNSRRData
import domain.model.DNSResourceRecord
import domain.model.enums.DNSKlass
import domain.model.enums.DNSMessageType
import domain.model.enums.DNSOpCode
import domain.model.enums.DNSQueryType
import domain.model.enums.DNSRCode
import extensions.lowestShort
import org.junit.Assert.assertEquals
import org.junit.Test
import java.nio.ByteBuffer
import kotlin.random.Random

class DNSPacketBuilderAndCompressorTest {

    @Test
    fun testFlags() {
        for (i in 0..100) {
            val expected = generateFlags()
            val compressed = DNSPacketCompressor.compress(expected)
            val actual = DNSPacketBuilder.buildFlags(compressed)
            assertEquals(expected, actual)
        }
    }

    @Test
    fun testHeader() {
        for (i in 0..1000) {
            val expected = generateHeader(Random.nextInt(), Random.nextInt())
            val compressed = DNSPacketCompressor.compress(expected)
            val actual = DNSPacketBuilder.buildHeader(ByteBuffer.wrap(compressed))
            assertEquals(expected, actual)
        }
    }

    @Test
    fun packetTest() {
        DNSQueryType.values().filter { it != DNSQueryType.UNDEFINED }.forEach { type ->
            runTestsWithType(type)
        }
    }

    private fun runTestsWithType(type: DNSQueryType) {
        println("runTestsWithType '$type' started")
        for (i in 0..100) {
            val expected = generatePacket(i, 0, type)
            val compressed = DNSPacketCompressor.compress(expected)
            val actual = DNSPacketBuilder.build(ByteBuffer.wrap(compressed))
            assertEquals(expected, actual)
        }
        for (i in 0..100) {
            val expected = generatePacket(0, i, type)
            val compressed = DNSPacketCompressor.compress(expected)
            val actual = DNSPacketBuilder.build(ByteBuffer.wrap(compressed))
            assertEquals(expected, actual)
        }
        println("runTestsWithType '$type' passed")
    }

    private fun generatePacket(numOfQueries: Int, numOfAnswers: Int, type: DNSQueryType) = DNSPacket(
            header = generateHeader(numOfQueries, numOfAnswers),
            queries = generateQueries(numOfQueries, type),
            answers = generateAnswers(numOfAnswers, type),
            authorityAnswers = emptyList(),
            additionalAnswers = emptyList()
    )

    private fun generateQueries(num: Int, type: DNSQueryType) = List(num) {
        DNSQuery(generateRandomDNSName(), type, DNSKlass.IN)
    }

    private fun generateAnswers(num: Int, type: DNSQueryType) = List(num) {
        val data = generateDNSRData(type)
        DNSResourceRecord(
                name = generateRandomDNSName(),
                type = type,
                klass = DNSKlass.IN,
                ttl = 1200,
                dataLength = data.getDataLength(),
                data = data
        )
    }

    private fun generateDNSRData(type: DNSQueryType) = when (type) {
        DNSQueryType.MX -> DNSRRData.MX(Random.nextInt().toShort(), generateRandomDNSName().toString())
        DNSQueryType.A -> DNSRRData.A(Random.nextInt())
        DNSQueryType.NS -> DNSRRData.NS(generateRandomDNSName().toString())
        DNSQueryType.CNAME -> DNSRRData.CName(generateRandomDNSName().toString())
        DNSQueryType.H_INFO -> DNSRRData.HInfo("AMD Ubuntu")
        else -> DNSRRData.A(Random.nextInt())
    }

    private fun generateRandomDNSName(): DNSName {
        val builder = DNSName.Builder()
        for (i in 1..Random.nextInt(2, 127)) {
            builder.addMark("a".repeat(Random.nextInt(1, 63)))
        }
        builder.addMark("")
        return builder.build()
    }

    private fun generateHeader(numOfQueries: Int, numOfAnswers: Int) = DNSHeader(
            id = Random.nextInt().lowestShort(),
            flags = generateFlags(),
            numOfQueries = numOfQueries.toShort(),
            numOfAnswers = numOfAnswers.toShort(),
            numOfAuthorityAnswers = 0,
            numOfAdditionalAnswers = 0
    )

    private fun generateFlags() = DNSFlags(
            qr = DNSMessageType.values().random(),
            opCode = DNSOpCode.STANDARD,
            aa = Random.nextBoolean(),
            tc = Random.nextBoolean(),
            rd = Random.nextBoolean(),
            ra = Random.nextBoolean(),
            rCode = DNSRCode.values().random()
    )

}
