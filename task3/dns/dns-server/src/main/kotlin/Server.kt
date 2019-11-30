import data.ConfigProvider
import domain.model.DNSFlags
import domain.model.DNSHeader
import domain.model.DNSPacket
import domain.model.DNSResourceRecord
import domain.model.enums.DNSMessageType
import domain.model.enums.DNSOpCode
import domain.model.enums.DNSRCode
import utils.DNSPacketBuilder
import utils.DNSPacketCompressor
import utils.inject
import java.net.InetSocketAddress
import java.nio.ByteBuffer
import java.nio.channels.DatagramChannel

class Server {

    companion object {
        private const val MAX_SIZE = 65_512
    }

    private val provider: ConfigProvider by inject()
    private val zoneInfo = provider.provideZoneInfo()

    private val channel = DatagramChannel.open().bind(InetSocketAddress(53))
    private val buffer = ByteBuffer.allocate(MAX_SIZE)

    fun start() {
        while (true) {
            val clientAddr = channel.receive(buffer)
            val receivedData = buffer.array().copyOfRange(0, buffer.position())
            val receivedPacket = DNSPacketBuilder.build(ByteBuffer.wrap(receivedData))
            val packet = wrap(receivedPacket.header.id, zoneInfo.first())
            channel.send(ByteBuffer.wrap(DNSPacketCompressor.compress(packet)), clientAddr)
            buffer.flip()
            println(packet)
        }
    }


    private fun wrap(id: Short, rr: DNSResourceRecord) = DNSPacket(
        header = DNSHeader(
            id = id,
            flags = DNSFlags(
                qr = DNSMessageType.ANSWER,
                opCode = DNSOpCode.STANDARD,
                aa = true,
                tc = false,
                ra = false,
                rd = false,
                rCode = DNSRCode.NO_ERROR
            ),
            numOfAnswers = 1,
            numOfQueries = 0,
            numOfAdditionalAnswers = 0,
            numOfAuthorityAnswers = 0
        ),
        queries = emptyList(),
        answers = listOf(rr),
        additionalAnswers = emptyList(),
        authorityAnswers = emptyList()
    )


}