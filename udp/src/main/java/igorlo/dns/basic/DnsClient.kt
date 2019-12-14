package igorlo.dns.basic

import igorlo.TextColors
import igorlo.util.UdpExchange
import igorlo.dns.message.DnsMessageBuilder
import igorlo.dns.message.DnsMessageClass
import igorlo.dns.message.MessageUtils
import igorlo.dns.message.Request
import igorlo.util.Utilities.colorPrint
import java.net.DatagramSocket
import java.net.Inet4Address
import java.util.*

class DnsClient {

    val socket = DatagramSocket()

    fun run() {
        Thread(Runnable {
            readingHandler()
        }).start()
        writingHandler()
    }

    private fun writingHandler() {
        val scanner = Scanner(System.`in`)
        var commandFirst: String
        while (true) {
            commandFirst = scanner.nextLine()
            val dnsQuerry = DnsMessageBuilder()
                    .setRandomId()
                    .addRequest(Request(commandFirst, 2, 1))
                    .build()
            colorPrint("--------SENT--------\n", TextColors.ANSI_CYAN)
            colorPrint(dnsQuerry.toString(), TextColors.ANSI_CYAN)
            colorPrint("--------------------\n", TextColors.ANSI_CYAN)
//            UdpExchange.sendUdp(dnsQuerry.rawMessage, UdpExchange.A_ROOT_SERVER_ADDRESS, socket, UdpExchange.DNS_PORT)
            UdpExchange.sendUdp(dnsQuerry.rawMessage, Inet4Address.getByAddress(byteArrayOf(1, 97, MessageUtils.unsignedByte(192), 42)), socket, UdpExchange.DNS_PORT)
        }
    }

    private fun readingHandler() {
        while (true) {
            val received = UdpExchange.recieveUdp(socket)
            val dnsAnswer = DnsMessageClass(received)
            colorPrint("------RECIEVED------\n", TextColors.ANSI_YELLOW)
            colorPrint(dnsAnswer.toString(), TextColors.ANSI_YELLOW)
            colorPrint("--------------------\n", TextColors.ANSI_YELLOW)
        }
    }

}