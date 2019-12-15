import java.io.File

import cats.effect.{Blocker, ExitCode, IO, IOApp, Resource}
import org.slf4j.{Logger, LoggerFactory}
import cats.syntax.functor._

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
            Rpc.upload(new File("test2.txt"), new File("test1.txt"))(channel, fs)
          }
      }
      .as(ExitCode.Success)

  val test = List(
      WRQ(new File("myFileName"))
    , RRQ(new File("secondFile"), Octet)
    , Acknowledgment(Block(123))
    , ErrorType(NoSuchUser, "There is an error")
    , Data(Block(7), "There is data incomming")
  )

  def check(p: Packet[Type], channel: SafeUdpChannel): IO[Unit] =
    p match {
      case Packet(ack: Acknowledgment, met) => channel.send[Acknowledgment](Packet(ack, met))
      case Packet(wrq: WRQ, met)            => channel.send[WRQ](Packet(wrq, met))
      case Packet(rrq: RRQ, met)            => channel.send[RRQ](Packet(rrq, met))
      case Packet(err: ErrorType, met)      => channel.send[ErrorType](Packet(err, met))
      case Packet(data: Data, met)          => channel.send[Data](Packet(data, met))
    }

}
