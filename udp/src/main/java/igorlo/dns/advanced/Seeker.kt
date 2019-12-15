package igorlo.dns.advanced

import igorlo.dns.message.*
import igorlo.util.UdpExchange
import java.net.DatagramSocket
import java.net.Inet4Address
import java.net.InetAddress
import java.net.SocketTimeoutException
import java.util.*

class Seeker(var socket: DatagramSocket, val rootAddress: InetAddress) {

    private var currentAddress = rootAddress
    private val toVisit: Stack<Pair<String, InetAddress>> = Stack()

    fun seek(domainName: String): Optional<Response> {
        toVisit.push(Pair("root", rootAddress))

        while (toVisit.isNotEmpty()) {
            socket = DatagramSocket()
            socket.soTimeout = 2000
            val nextPair = toVisit.pop()
            currentAddress = nextPair.second
            println("$currentAddress - ${nextPair.first}")
            val optionalResult = askServer(currentAddress, domainName)
            if (optionalResult.isEmpty){
                println("Сервер не ответил")
                continue
            }
            val response = optionalResult.get()
            if (response.responseQuantity > 0) {
                return Optional.of(response.responses.first())
            } else if (response.additionalInfoQuantity > 0) {
                for (additional in response.additionalResponses) {
                    if (additional.type == DnsType.A.type || additional.type == DnsType.NS.type) {
                        toVisit.push(
                                Pair(additional.address, Inet4Address.getByAddress(additional.rData))
                        )
                    }
                }
            } else {
                continue
            }
        }
        return Optional.empty()
    }

    private fun askServer(address: InetAddress, domainName: String): Optional<DnsMessage> {
        val querry = DnsMessageBuilder()
                .addRequestTo(domainName)
                .setRandomId()
                .build()
        UdpExchange.sendUdp(querry.rawMessage, address, socket, UdpExchange.DNS_PORT)
        try {
            val responseBytes = UdpExchange.recieveUdp(socket)
            return Optional.of(DnsMessageClass(responseBytes))
        } catch (e: SocketTimeoutException) {
            return Optional.empty()
        }
    }

}