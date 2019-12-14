package igorlo.dns.advanced

import igorlo.util.UdpExchange
import igorlo.util.Utilities
import java.net.DatagramSocket

object Main {

    @JvmStatic
    fun main(args: Array<String>) {
        val datagramSocket = DatagramSocket()
        val seeker = Seeker(datagramSocket, UdpExchange.С_ROOT_SERVER_ADDRESS)
        val optionalAnswer = seeker.seek("microsoft.com")
        if (optionalAnswer.isPresent) {
            println("Вау, мы нашли его:\n")
            val ipAddress: ByteArray = optionalAnswer.get().rData
            println(Utilities.ipAddressToString(ipAddress))
        }
    }

}