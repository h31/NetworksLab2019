package igorlo.dns.basic

import igorlo.TextColors
import igorlo.dns.advanced.Seeker
import igorlo.util.UdpExchange
import igorlo.dns.message.DnsMessageBuilder
import igorlo.dns.message.DnsMessageClass
import igorlo.dns.message.Request
import igorlo.util.Utilities.colorPrint
import java.net.DatagramSocket
import java.net.Inet4Address
import java.util.*
import kotlin.system.exitProcess

class SeekerClient {

    val socket = DatagramSocket()
    val seeker = Seeker(socket, UdpExchange.A_ROOT_SERVER_ADDRESS)

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
            if (commandFirst == "exit"){
                exitProcess(0)
            }
            val response = seeker.seek(commandFirst)
            if (response.isEmpty) {
                colorPrint("Домен $commandFirst не найден!\n", TextColors.ANSI_RED)
            } else {
                colorPrint(
                        "Домен $commandFirst Найден!\n" +
                                "Его IP: ${Inet4Address.getByAddress(response.get().rData)}\n",
                        TextColors.ANSI_GREEN
                )
            }
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
