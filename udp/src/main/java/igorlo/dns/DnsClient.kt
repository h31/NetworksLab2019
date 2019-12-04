package igorlo.dns

import igorlo.TextColors
import igorlo.dns.message.DnsMessageBuilder
import igorlo.dns.message.DnsMessageClass
import igorlo.util.Utilities.colorPrint
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.Inet4Address
import java.net.InetAddress
import java.util.*

class DnsClient {

    val socket = DatagramSocket()
    val address: InetAddress = Inet4Address.getByAddress(byteArrayOf(8, 8, 8, 8))
    val dnsPort = 53

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
                    .addRequestTo(commandFirst)
                    .build()
            val packet = DatagramPacket(dnsQuerry.rawMessage, dnsQuerry.rawMessage.size, address, dnsPort)
            colorPrint("--------SENT--------\n", TextColors.ANSI_CYAN)
            colorPrint(dnsQuerry.toString(), TextColors.ANSI_CYAN)
            colorPrint("--------------------\n", TextColors.ANSI_CYAN)
            socket.send(packet)
        }
    }

    private fun readingHandler() {
        while (true) {
            val readBuffer = ByteArray(512)
            val packet = DatagramPacket(readBuffer, readBuffer.size)
            socket.receive(packet)
            val received = packet.data
            val dnsAnswer = DnsMessageClass(received)
            colorPrint("------RECIEVED------\n", TextColors.ANSI_YELLOW)
            colorPrint(dnsAnswer.toString(), TextColors.ANSI_YELLOW)
            colorPrint("--------------------\n", TextColors.ANSI_YELLOW)
        }
    }

}