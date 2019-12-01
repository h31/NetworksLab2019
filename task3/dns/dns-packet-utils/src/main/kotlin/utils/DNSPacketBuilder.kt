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

    fun build(byteBuffer: ByteBuffer): DNSPacket {
        val header = buildHeader(byteBuffer)
        return DNSPacket(
                header = header,
                queries = List(header.numOfQueries.toInt()) { buildQuery(byteBuffer) },
                answers = List(header.numOfAnswers.toInt()) { buildResourceRecord(byteBuffer) },
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

    internal fun buildQuery(byteBuffer: ByteBuffer) = DNSQuery(
            name = buildName(byteBuffer),
            type = DNSQueryType.of(byteBuffer.consumeShort()),
            klass = DNSKlass.of(byteBuffer.consumeShort())
    )

    internal fun buildResourceRecord(byteBuffer: ByteBuffer): DNSResourceRecord {
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
                data = buildData(byteBuffer, dataLength, DNSQueryType.of(type))
        )
    }

    internal fun buildData(byteBuffer: ByteBuffer, dataLength: Short, type: DNSQueryType) = when (type) {
        DNSQueryType.A -> DNSRRData.A(byteBuffer.consumeInt())
        DNSQueryType.CNAME -> DNSRRData.CName(String(byteBuffer.consume(dataLength)))
        DNSQueryType.H_INFO -> DNSRRData.HInfo(String(byteBuffer.consume(dataLength)))
        DNSQueryType.NS -> DNSRRData.NS(String(byteBuffer.consume(dataLength)))
        DNSQueryType.MX -> DNSRRData.MX(
                byteBuffer.consumeShort(),
                String(byteBuffer.consume(dataLength - Short.SIZE_BYTES)).trim()
        )
        else -> DNSRRData.Undefined
    }

    internal fun buildName(byteBuffer: ByteBuffer): DNSName {
        val builder = DNSName.Builder()
        var markSize = byteBuffer.consumeByte()
        while (markSize.toInt() != 0) {
            builder.addMark(String(byteBuffer.consume(markSize.toShort())))
            markSize = byteBuffer.consumeByte()
        }
        builder.addMark("")
        return builder.build()
    }

}
