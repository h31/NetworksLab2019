import cats.effect.{Blocker, ExitCode, IO, IOApp}
import org.slf4j.LoggerFactory

object Application extends IOApp {
  implicit val logger = LoggerFactory.getLogger(getClass)
  val blocker         = Blocker[IO]

  def run(args: List[String]): IO[ExitCode] =
    blocker.use { blocker =>
      Server.run(blocker)
    }
}
