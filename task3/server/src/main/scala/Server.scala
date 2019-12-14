import cats.effect.{ExitCode, IO}
import cats.syntax.flatMap._

object Server {
  def run(port: Int = 69): IO[ExitCode] =
    SafeUdpChannel(port).use { channel =>
      def eventLoop: IO[ExitCode] = channel.receive.flatMap(print) >> eventLoop
      eventLoop
    }

  def print(p: Packet[Type]): IO[Unit] =
    p match {
      case Packet(ack: Acknowledgment, met) =>
        IO { println(s"type: ack, ${ack.block}, ${met.port}, ${met.addr}") }
      case Packet(wrq: WRQ, met) =>
        IO { println(s"type: wrq, ${wrq.file}, ${wrq.mode.value}, ${met.port}, ${met.addr}"); }
      case Packet(rrq: RRQ, met) =>
        IO { println(s"type: rrq, ${rrq.file}, ${rrq.mode.value}, ${met.port}, ${met.addr}") }
      case Packet(err: ErrorType, met) =>
        IO {
          println(
              s"type: err, ${err.errorCode.info}, ${err.errorCode.value}, ${err.errorMsg}, ${met.port}, ${met.addr}"
          )
        }
      case Packet(data: Data, met) =>
        IO { println(s"type: data, ${data.block}, ${data.data}, ${met.port}, ${met.addr}") }
    }

}
