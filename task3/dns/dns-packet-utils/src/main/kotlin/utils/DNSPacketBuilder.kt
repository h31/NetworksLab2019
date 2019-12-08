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
import extensions.consume
import extensions.consumeByte
import extensions.consumeInt
import extensions.consumeShort
import java.nio.ByteBuffer
import kotlin.experimental.and

// http://kunegin.com/ref3/dns/format.htm

object DNSPacketBuilder {

    private const val OP_CODE_OFFSET = 11

    private const val QR_MASK = 0x8000.toShort()
    private const val OP_CODE_MASK = 0x7800.toShort()
    private const val AA_MASK = 0x0400.toShort()
    private const val TC_MASK = 0x0200.toShort()
    private const val RD_MASK = 0x0100.toShort()
    private const val RA_MASK = 0x0080.toShort()
    private const val R_CODE_MASK = 0x000F.toShort()

    private const val NAME_PTR_MASK = 0xC0.toByte()
    private const val NAME_PTR_REVERSED_MASK = 0x3F.toByte()
    private const val RESERVED_MARK_MASK1 = 0x80.toByte()
    private const val RESERVED_MARK_MASK2 = 0x40.toByte()

    fun build(byteBuffer: ByteBuffer): DNSPacket {
        val header = buildHeader(byteBuffer)
        val queries = List(header.numOfQueries.toInt()) { buildQuery(byteBuffer) }
        val answers = List(header.numOfAnswers.toInt()) { buildResourceRecord(byteBuffer) }
        return DNSPacket(
                header = header,
                queries = queries,
                answers = answers,
                authorityAnswers = List(header.numOfAuthorityAnswers.toInt()) { buildResourceRecord(byteBuffer) },
                additionalAnswers = List(header.numOfAdditionalAnswers.toInt()) { buildResourceRecord(byteBuffer) }
        )
    }

    internal fun buildFlags(value: Short) = DNSFlags(
            qr = DNSMessageType.of(value.and(QR_MASK).toInt() == 0),
            opCode = DNSOpCode.of(value.and(OP_CODE_MASK).toInt().shr(OP_CODE_OFFSET).toByte()),
            aa = value.and(AA_MASK).toInt() != 0,
            tc = value.and(TC_MASK).toInt() != 0,
            rd = value.and(RD_MASK).toInt() != 0,
            ra = value.and(RA_MASK).toInt() != 0,
            rCode = DNSRCode.of(value.and(R_CODE_MASK).toByte())
    )

    internal fun buildHeader(byteBuffer: ByteBuffer) = DNSHeader(
            id = byteBuffer.consumeShort(),
            flags = buildFlags(byteBuffer.consumeShort()),
            numOfQueries = byteBuffer.consumeShort(),
            numOfAnswers = byteBuffer.consumeShort(),
            numOfAuthorityAnswers = byteBuffer.consumeShort(),
            numOfAdditionalAnswers = byteBuffer.consumeShort()
    )

    private fun buildQuery(byteBuffer: ByteBuffer) = DNSQuery(
            name = buildName(byteBuffer),
            type = DNSQueryType.of(byteBuffer.consumeShort()),
            klass = DNSKlass.of(byteBuffer.consumeShort())
    )

    private fun buildResourceRecord(byteBuffer: ByteBuffer): DNSResourceRecord {
        val name = buildName(byteBuffer)
        val type = byteBuffer.consumeShort()
        val klass = DNSKlass.of(byteBuffer.consumeShort())
        val ttl = byteBuffer.consumeInt()
        val dataLength = byteBuffer.consumeShort()
        return DNSResourceRecord(
                name = name,
                type = DNSQueryType.of(type),
                klass = klass,
                ttl = ttl,
                dataLength = dataLength,
                data = buildData(byteBuffer, DNSQueryType.of(type))
        )
    }

    private fun buildData(byteBuffer: ByteBuffer, type: DNSQueryType) = when (type) {
        DNSQueryType.A -> DNSRRData.A(byteBuffer.consumeInt())
        DNSQueryType.CNAME -> DNSRRData.CName(buildName(byteBuffer))
        DNSQueryType.H_INFO -> DNSRRData.HInfo(buildName(byteBuffer))
        DNSQueryType.NS -> DNSRRData.NS(buildName(byteBuffer))
        DNSQueryType.MX -> DNSRRData.MX(
                byteBuffer.consumeShort(),
                buildName(byteBuffer)
        )
        else -> DNSRRData.Undefined
    }

    private fun buildName(byteBuffer: ByteBuffer): DNSName {
        var markSize = byteBuffer.consumeByte()
        if (markSize.and(NAME_PTR_MASK) != 0.toByte()) {
            return findNameByPtr(markSize, byteBuffer.consumeByte(), byteBuffer)
        }
        if (markSize.and(RESERVED_MARK_MASK1) != 0.toByte() || markSize.and(RESERVED_MARK_MASK2) != 0.toByte()) {
            throw IllegalArgumentException("Found reserved mark combination 10 or 01")
        }
        val builder = DNSName.Builder()
        while (markSize.toInt() != 0) {
            builder.addMark(String(byteBuffer.consume(markSize.toShort())))
            markSize = byteBuffer.consumeByte()
        }
        builder.addMark("")
        return builder.build()
    }

    private fun findNameByPtr(firstOctet: Byte, secondOctet: Byte, byteBuffer: ByteBuffer): DNSName {
        var markPosition = (firstOctet and NAME_PTR_REVERSED_MASK).toInt().shl(Byte.SIZE_BITS) + secondOctet.toInt()
        val builder = DNSName.Builder()
        var markSize = byteBuffer.get(markPosition)
        while (markSize.toInt() != 0) {
            builder.addMark(String(byteBuffer.array().copyOfRange(markPosition, markPosition + markSize + 1)))
            markPosition += markSize + 1
            markSize = byteBuffer.get(markPosition)
        }
        return builder.build()
    }

}
