import java.io.File
import java.net.InetAddress
import cats.implicits._
import cats.effect.{ExitCode, IO, IOApp}

object Application extends IOApp {
  override def run(args: List[String]): IO[ExitCode] =
    SafeUdpChannel(1666)
      .use { channel =>
        test
          .map(x => Packet(x, Metadata(InetAddress.getLocalHost)))
          .traverse { element =>
            check(element, channel)
          }
          .void
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
