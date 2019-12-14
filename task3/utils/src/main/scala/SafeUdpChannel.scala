import java.net.{DatagramPacket, DatagramSocket}

import Codec.{Decoder, Encoder}
import cats.effect.{IO, Resource}
import cats.syntax.functor._

/**
  * Represents abstraction over udp socket, that
  * use tftp-primitives for communication.
  */
trait Channel[T, F[A <: T]] {

  /**
    * Receive data from channel in a safe manner
    */
  def receive: IO[Option[F[_]]]

  /**
    *
    */
  def send[S <: T](packet: F[S])(implicit e: Encoder[S]): IO[Unit]
}

class SafeUdpChannel private (socket: DatagramSocket) extends Channel[Type, Packet] {
  private val buffer = IO(Buffer(65535))

  def receive: IO[Option[Packet[Type]]] =
    buffer
      .flatMap { bf =>
        val datagram = new DatagramPacket(bf.array, bf.size)
        IO(socket.receive(datagram)).as(datagram)
      }
      .map(PacketHandler.handle)

  def send[S <: Type](tftpPacket: Packet[S])(implicit e: Encoder[S]): IO[Unit] =
    IO {
      socket.send {
        val data = e.encode(tftpPacket.packet)
        new DatagramPacket(data, data.length, tftpPacket.metadata.addr, tftpPacket.metadata.port)
      }
    }

  private object PacketHandler {

    /**
      * Bad code, but i don't want use reflection
      */
    def handle(datagram: DatagramPacket): Option[Packet[Type]] = {
      val data = Buffer.wrap(datagram.getData)
      (data.takeNumber match {
        case 1 => Decoder.decode[RRQ](data.array)
        case 2 => Decoder.decode[WRQ](data.array)
        case 3 => Decoder.decode[Data](data.array)
        case 4 => Decoder.decode[Acknowledgment](data.array)
        case 5 => Decoder.decode[ErrorType](data.array)
      }).toOption
        .map(t => Packet(t, Metadata(datagram.getAddress, datagram.getPort)))
    }
  }
}

object SafeUdpChannel {
  def apply(port: Int): Resource[IO, SafeUdpChannel] =
    Resource.fromAutoCloseable(IO(new DatagramSocket(port))).map { socket =>
      new SafeUdpChannel(socket)
    }
}
