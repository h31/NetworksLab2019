package igorlo.dns.advanced

import igorlo.dns.basic.SeekerClient
import igorlo.util.UdpExchange
import igorlo.util.Utilities
import java.net.DatagramSocket

object Main {

    @JvmStatic
    fun main(args: Array<String>) {
        SeekerClient().run()
    }

}