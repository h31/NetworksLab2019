package service.impl

import domain.model.DNSName
import domain.model.DNSResult
import domain.model.enums.DNSQueryType
import service.DNSService
import utils.DNSPacketBuilder
import utils.DNSPacketCompressor
import utils.DNSQueryPacketBuilder
import utils.InetAddressResolver
import java.net.InetSocketAddress
import java.nio.ByteBuffer
import java.nio.channels.DatagramChannel
import kotlin.random.Random

class DNSServiceImpl : DNSService {

    companion object {
        private const val TIMEOUT_MS = 5_000L
        private const val RETRY_TIME = 3
    }

    private val buffer = ByteBuffer.allocate(65_512)

    override fun makeRequest(type: DNSQueryType, dnsServerAddress: String, dnsName: DNSName): DNSResult {
        val channel = DatagramChannel.open()
        return try {
            val inetAddress = InetAddressResolver.resolve(dnsServerAddress)
            val id = Random.nextInt().toShort()
            val packet = DNSQueryPacketBuilder.buildDNSQueryPacket(id, type, dnsName)
            channel.send(ByteBuffer.wrap(DNSPacketCompressor.compress(packet)), InetSocketAddress(inetAddress, 53))
            channel.receive(buffer)
            val receivedData = buffer.array().copyOfRange(0, buffer.position())
            DNSResult.Data(DNSPacketBuilder.build(ByteBuffer.wrap(receivedData)))
        } catch (e: IllegalArgumentException) {
            DNSResult.Error(e.message ?: "")
        } finally {
            channel.close()
        }
    }

}
