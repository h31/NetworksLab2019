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
}
