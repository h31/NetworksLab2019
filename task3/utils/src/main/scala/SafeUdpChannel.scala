import java.net.{DatagramPacket, DatagramSocket}

import Codec.{Decoder, Encoder}
import cats.effect.{IO, Resource}
import cats.syntax.functor._
import cats.syntax.flatMap._
import org.slf4j.Logger

/**
  * Represents abstraction over udp socket, that
  * use tftp-primitives for communication.
  */
trait Channel[T, F[A <: T]] {

  /**
    * Receive data from channel in a safe manner
    */
  def receive: IO[F[_]]

  /**
    * Send data to channel and encode it using
    *
    */
  def send[S <: T](packet: F[S])(implicit e: Encoder[S]): IO[Unit]
}

class SafeUdpChannel private (socket: DatagramSocket)(implicit logger: Logger) extends Channel[Type, Packet] {
  private val buffer = IO(Buffer(capacity = 65535))

  def receive: IO[Packet[Type]] =
    buffer
      .flatMap { bf =>
        val datagram = new DatagramPacket(bf.array, bf.size)
        IO(socket.receive(datagram)).as(datagram)
      }
      .map(PacketHandler.handle)
      .flatTap(x => IO(logger.info(x.toString)))

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
    def handle(datagram: DatagramPacket): Packet[Type] = {
      val data     = Buffer.wrap(datagram.getData)
      val metadata = Metadata(datagram.getAddress, datagram.getPort)
      (data.takeNumber match {
        case 1 => Decoder.decode[RRQ](data.array)
        case 2 => Decoder.decode[WRQ](data.array)
        case 3 => Decoder.decode[Data](data.array)
        case 4 => Decoder.decode[Acknowledgment](data.array)
        case 5 => Decoder.decode[ErrorType](data.array)
      }).fold[Packet[Type]](
          _ => Packet(ErrorType(NotDefined, "error during decoding"), metadata)
        , Packet(_, metadata)
      )
    }
  }
}

object SafeUdpChannel {
  def apply(port: Int)(implicit logger: Logger): Resource[IO, SafeUdpChannel] =
    Resource.fromAutoCloseable(IO(new DatagramSocket(port))).map { socket =>
      new SafeUdpChannel(socket)
    }
}
