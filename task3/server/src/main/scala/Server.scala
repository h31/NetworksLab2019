import cats.effect.{Blocker, ContextShift, ExitCode, IO}
import cats.syntax.flatMap._
import org.slf4j.Logger
import cats.syntax.functor._

object Server {
  def run(blocker: Blocker, port: Int = 69)(implicit logger: Logger, ctx: ContextShift[IO]): IO[ExitCode] =
    (for {
      cache      <- Cache[IO]()
      fileSystem <- FileSystem(blocker)
    } yield (cache, fileSystem))
      .flatMap {
        case (cache, fs) =>
          SafeUdpChannel(port).use { channel =>
            def eventLoop: IO[Unit] =
              channel.receive.flatMap(StateMachine.next(_, channel)(fs, cache)) >> eventLoop

            eventLoop
          }
      }
      .as(ExitCode.Success)

  def print(p: Packet[Type]): IO[Unit] =
    p match {
      case Packet(ack: Acknowledgment, met) =>
        IO { println(s"type: ack, ${ack.block}, ${met.port}, ${met.addr}") }
      case Packet(wrq: WRQ, met) =>
        IO { println(s"type: wrq, ${wrq.file}, ${wrq.mode.value}, ${met.port}, ${met.addr}") }
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
