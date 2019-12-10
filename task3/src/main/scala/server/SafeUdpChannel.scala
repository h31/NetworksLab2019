package server

import java.net.{DatagramPacket, DatagramSocket}
import java.nio.ByteBuffer

import cats.effect.{IO, Resource}
import server.Codec.{Decoded, Encoded}
import utils.Packet

import scala.collection.mutable

trait Channel {

  /**
    * Represents request from client, that will
    * be decoded using Decoded typeclass
    */
  def request[T <: Packet](implicit d: Decoded[T]): IO[T]

  /**
    * Represents response to client, that will
    *  be encoded using Encoded typeclass
    */
  def response[T <: Packet](packet: T)(implicit e: Encoded[T]): IO[Unit]
}

class SafeUdpChannel private (socket: DatagramSocket) extends Channel {
  private val size = Int.MaxValue / 2

  private val buffer = IO(ByteBuffer.allocate(size))

  override def request[T <: Packet](implicit d: Decoded[T]): IO[T] =
    for {
      bf <- buffer
      datagram = new DatagramPacket(bf.array, bf.capacity)
      _ <- IO(socket.receive(datagram))
    } yield ???

  override def response[T <: Packet](packet: T)(implicit e: Encoded[T]): IO[Unit] = ???
}

object SafeUdpChannel {
  def apply(port: Int): IO[SafeUdpChannel] =
    Resource.fromAutoCloseable(IO(new DatagramSocket(port))).use { socket =>
      IO(new SafeUdpChannel(socket))
    }
}
