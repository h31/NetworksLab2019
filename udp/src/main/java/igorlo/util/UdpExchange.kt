package igorlo.util

import igorlo.dns.message.MessageUtils
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.Inet4Address
import java.net.InetAddress

object UdpExchange {

    const val DNS_PORT = 53
    val A_ROOT_SERVER_ADDRESS: InetAddress = Inet4Address.getByAddress(byteArrayOf(MessageUtils.unsignedByte(198), 41, 0, 4))
    val С_ROOT_SERVER_ADDRESS: InetAddress = Inet4Address.getByAddress(byteArrayOf(MessageUtils.unsignedByte(192), 33, 4, 12))
    val GOOGLE_DNS_ADDRESS: InetAddress = Inet4Address.getByAddress(byteArrayOf(8, 8, 8, 8))


    fun sendUdp(message: ByteArray, address: InetAddress, socket: DatagramSocket, port: Int) {
        val packet = DatagramPacket(message, message.size, address, port)
        socket.send(packet)
    }

    fun recieveUdp(socket: DatagramSocket): ByteArray {
        val readBuffer = ByteArray(512)
        val packet = DatagramPacket(readBuffer, readBuffer.size)
        socket.receive(packet)
        return packet.data
    }

}