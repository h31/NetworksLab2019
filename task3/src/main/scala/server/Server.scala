package server

import java.net.{DatagramPacket, DatagramSocket}

import cats.effect.{IO, Resource}

object Server {
  def run(port: Int = 69): IO[Unit] = {
    /* Infinite loop */
    val eventLoop: SafeUdpChannel => IO[Unit] = ???

    SafeUdpChannel(port).flatMap(ch => eventLoop(ch))
  }
}
