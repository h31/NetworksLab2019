import cats.effect.{ExitCode, IO, IOApp}

object Application extends IOApp {
  def run(args: List[String]): IO[ExitCode] = Server.run()
}
