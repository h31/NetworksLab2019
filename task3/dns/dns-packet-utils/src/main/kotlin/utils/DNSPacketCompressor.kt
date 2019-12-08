package utils

import domain.model.DNSFlags
import domain.model.DNSHeader
import domain.model.DNSName
import domain.model.DNSPacket
import domain.model.DNSQuery
import domain.model.DNSRRData
import domain.model.DNSResourceRecord
import extensions.highestByte
import extensions.highestShort
import extensions.lowestByte
import extensions.lowestShort
import kotlin.experimental.and

object DNSPacketCompressor {

    fun compress(packet: DNSPacket): ByteArray = with(packet) {
        compress(header) +
                reduceQueriesList(queries) +
                reduceRRList(answers) +
                reduceRRList(authorityAnswers) +
                reduceRRList(additionalAnswers)
    }

    internal fun compress(query: DNSQuery): ByteArray = with(query) {
        compress(name) + arrayOf(
                type.value.highestByte(),
                type.value.lowestByte(),
                klass.value.highestByte(),
                klass.value.lowestByte()
        ).toByteArray()
    }

    internal fun compress(resourceRecord: DNSResourceRecord): ByteArray = with(resourceRecord) {
        compress(name) + arrayOf(
                type.value.highestByte(),
                type.value.lowestByte(),
                klass.value.highestByte(),
                klass.value.lowestByte(),
                ttl.highestShort().highestByte(),
                ttl.highestShort().lowestByte(),
                ttl.lowestShort().highestByte(),
                ttl.lowestShort().lowestByte(),
                dataLength.highestByte(),
                dataLength.lowestByte()
        ).toByteArray() + compress(data)
    }

    internal fun compress(data: DNSRRData) = when (data) {
        is DNSRRData.A -> arrayOf(
                data.address.highestShort().highestByte(),
                data.address.highestShort().lowestByte(),
                data.address.lowestShort().highestByte(),
                data.address.lowestShort().lowestByte()
        ).toByteArray()
        is DNSRRData.NS -> compress(data.name)
        is DNSRRData.HInfo -> compress(data.cpuAndOs)
        is DNSRRData.CName -> compress(data.name)
        is DNSRRData.MX -> arrayOf(
                data.preference.highestByte(),
                data.preference.lowestByte()
        ).toByteArray() + compress(data.exchange)
        is DNSRRData.Undefined -> ByteArray(0)
    }

    internal fun compress(name: DNSName): ByteArray = name.marks
            .map {
                arrayOf(it.length.toByte()).toByteArray() + it.toByteArray()
            }
            .reduce(ByteArray::plus)

    internal fun compress(header: DNSHeader): ByteArray = with(header) {
        val flags = compress(header.flags)
        arrayOf(
                id.highestByte(),
                id.lowestByte(),
                flags.highestByte(),
                flags.lowestByte(),
                numOfQueries.highestByte(),
                numOfQueries.lowestByte(),
                numOfAnswers.highestByte(),
                numOfAnswers.lowestByte(),
                numOfAuthorityAnswers.highestByte(),
                numOfAuthorityAnswers.lowestByte(),
                numOfAdditionalAnswers.highestByte(),
                numOfAdditionalAnswers.lowestByte()
        ).toByteArray()
    }

    internal fun compress(flags: DNSFlags): Short = with(flags) {
        var res = 0
        if (qr.value) res += 1
        res = res.shl(4)
        res += opCode.value.and(0x0F)
        res = res.shl(1)
        if (aa) res += 1
        res = res.shl(1)
        if (tc) res += 1
        res = res.shl(1)
        if (rd) res += 1
        res = res.shl(1)
        if (ra) res += 1
        res = res.shl(7) // 3 reserve + 4 for rCode
        res += rCode.value.and(0x0F)
        res.toShort()
    }

    private fun reduceQueriesList(queries: List<DNSQuery>) = if (queries.isNotEmpty()) {
        queries.map(::compress).reduce(ByteArray::plus)
    } else {
        byteArrayOf()
    }

    private fun reduceRRList(rrList: List<DNSResourceRecord>) = if (rrList.isNotEmpty()) {
        rrList.map(::compress).reduce(ByteArray::plus)
    } else {
        byteArrayOf()
    }

}
