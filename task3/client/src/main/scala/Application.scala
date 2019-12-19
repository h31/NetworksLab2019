import java.io.File

import cats.effect.{Blocker, ExitCode, IO, IOApp, Resource}
import org.slf4j.{Logger, LoggerFactory}
import cats.syntax.functor._
import cats.syntax.flatMap._

import scala.io.StdIn

object Application extends IOApp {
  implicit val logger: Logger = LoggerFactory.getLogger(getClass)
  val blocker                 = Blocker[IO]

  override def run(args: List[String]): IO[ExitCode] =
    SafeUdpChannel(1234)
      .flatMap(
          channel =>
          blocker
            .flatMap(bl => Resource.pure((channel, bl)))
      )
      .use {
        case (channel, bl) =>
          FileSystem(bl).flatMap { fs =>
            def readLoop: IO[ExitCode] =
              getStr().flatMap {
                case "upload" =>
                  sourceAndPath.flatMap {
                    case (name, path) =>
                      Rpc.upload(new File(path), new File(name))(channel, fs) >>
                        IO(logger.info("Uploaded succesfully"))
                  } >> readLoop
                case "download" =>
                  getStr().flatMap(str => Rpc.download(new File(str))(channel, fs)) >> IO(
                      logger.info("Downloaded succesfully")
                  ) >> readLoop
                case _ => readLoop
              }
            readLoop
          }
      }
      .as(ExitCode.Success)

  def sourceAndPath: IO[(String, String)] =
    for {
      name <- getStr()
      path <- getStr()
    } yield (name, path)

  def putStr(line: String): IO[Unit] = IO(println(line))
  def getStr(): IO[String]           = IO(StdIn.readLine())
}
