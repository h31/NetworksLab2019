package service.impl

import domain.model.DNSName
import domain.model.DNSPacket
import domain.model.DNSResult
import domain.model.enums.DNSQueryType
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import service.DNSService
import utils.DNSPacketBuilder
import utils.DNSPacketCompressor
import utils.DNSQueryPacketBuilder
import utils.InetAddressResolver
import java.net.InetAddress
import java.net.InetSocketAddress
import java.nio.ByteBuffer
import java.nio.channels.DatagramChannel
import kotlin.random.Random

class DNSServiceImpl : DNSService {

    companion object {
        private const val TIMEOUT_MS = 5_000L
        private const val DNS_SERVER_PORT = 53
        private const val MAX_PACKET_SIZE = 65_512
        private const val RETRY_TIME = 3
    }

    private lateinit var job: Job
    private val channels = mutableListOf<DatagramChannel>()

    override fun makeRequest(
            type: DNSQueryType,
            dnsServerAddress: String,
            dnsName: DNSName,
            onResult: (DNSResult) -> Unit
    ) {
        try {
            val inetAddress = InetAddressResolver.resolve(dnsServerAddress)
            val id = Random.nextInt().toShort()
            val packet = DNSQueryPacketBuilder.buildDNSQueryPacket(id, type, dnsName)
            sendPacketAndGetResult(packet, inetAddress, onResult)
        } catch (e: Throwable) {
            onResult.invoke(DNSResult.Error(e.message ?: ""))
        } finally {
            channels.forEach(DatagramChannel::close)
        }
    }

    private fun sendPacketAndGetResult(
            packet: DNSPacket,
            inetAddress: InetAddress,
            onResult: (DNSResult) -> Unit
    ) {
        val byteBuffer = ByteBuffer.wrap(DNSPacketCompressor.compress(packet))
        var lastJob: Job? = null
        job = GlobalScope.launch(Dispatchers.IO) {
            repeat(RETRY_TIME) {
                lastJob?.cancel()
                val channel = DatagramChannel.open()
                channels += channel
                println("Sending a query packet...")
                channel.send(
                        byteBuffer,
                        InetSocketAddress(inetAddress, DNS_SERVER_PORT)
                )
                lastJob = waitForResult(channel, onResult)
                delay(TIMEOUT_MS)
            }
            onResult.invoke(DNSResult.Error("Can't receive result"))
        }
        runBlocking {
            job.join()
        }
    }

    private fun waitForResult(channel: DatagramChannel, onResult: (DNSResult) -> Unit) =
            GlobalScope.launch(Dispatchers.IO) {
                val buffer = ByteBuffer.allocate(MAX_PACKET_SIZE)
                channel.receive(buffer)
                val receivedData = buffer.array().copyOfRange(0, buffer.position())
                onResult.invoke(DNSResult.Data(DNSPacketBuilder.build(ByteBuffer.wrap(receivedData))))
                job.cancel()
            }

}
