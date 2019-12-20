package igorlo.util

import igorlo.dns.message.MessageUtils
import java.net.DatagramPacket
import java.net.DatagramSocket
import java.net.Inet4Address
import java.net.InetAddress

object UdpExchange {

    const val DNS_PORT = 53
    const val BYTES_TO_READ = 1024
    val A_ROOT_SERVER_ADDRESS: InetAddress = Inet4Address.getByAddress(byteArrayOf(MessageUtils.unsignedByte(198), 41, 0, 4))
    val С_ROOT_SERVER_ADDRESS: InetAddress = Inet4Address.getByAddress(byteArrayOf(MessageUtils.unsignedByte(192), 33, 4, 12))
    val MIRROR_ROOT_SERVER_ADDRESS: InetAddress = Inet4Address.getByAddress(byteArrayOf(MessageUtils.unsignedByte(199), 7, 83, 42))
    val GOOGLE_DNS_ADDRESS: InetAddress = Inet4Address.getByAddress(byteArrayOf(8, 8, 8, 8))


    fun sendUdp(message: ByteArray, address: InetAddress, socket: DatagramSocket, port: Int) {
        val packet = DatagramPacket(message, message.size, address, port)
        socket.send(packet)
    }

    fun recieveUdp(socket: DatagramSocket): ByteArray {
        val readBuffer = ByteArray(BYTES_TO_READ)
        val packet = DatagramPacket(readBuffer, readBuffer.size)
        socket.receive(packet)
        return packet.data
    }

}