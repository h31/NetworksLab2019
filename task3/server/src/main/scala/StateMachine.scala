import java.io.File

import cats.effect.IO
import cats.syntax.flatMap._
import fs2._

object StateMachine {
  def next(p: Packet[Type], channel: Channel[Type, Packet])(fs: FileSystem, cache: Cache[IO]): IO[Unit] = {
    def exists(tp: IOType)(no: => IO[Unit])(yes: => IO[Unit]): IO[Unit] =
      fs.exists(tp.file).flatMap { maybe =>
        maybe.fold(no)(_ => yes)
      }

    def check(port: Int)(no: => IO[Unit])(fn: File => IO[Unit]): IO[Unit] =
      cache.file(port).flatMap(_.fold(no)(fn))

    p match {
      case Packet(wrq: WRQ, met) =>
        exists(wrq)(cache.add(met.port -> wrq.file) >> channel.send(Packet(Acknowledgment(Block(0)), met))) {
          channel.send {
            Packet(ErrorType(AlreadyExists, s"file with name: ${wrq.file.getName} already exist"), met)
          }
        }

      case Packet(rrq: RRQ, met) =>
        exists(rrq)(channel.send(Packet(ErrorType(FileNotFound, s"no such file: ${rrq.file.getName}"), met))) {
          cache.add(met.port -> rrq.file) >> fs
            .readChunk(rrq.file, Block(1))
            .map(data => Packet(Data(Block(1), data), met))
            .compile
            .lastOrError
            .flatMap(channel.send[Data])
        }

      case Packet(data: Data, met) =>
        cache
          .file(met.port)
          .flatMap(
              _.fold(channel.send(Packet(ErrorType(NoSuchUser, "no such user is session cache"), met)))(
                file =>
                (fs.writeChunk(file, data.data) ++ Stream.emit(Packet(Acknowledgment(data.block), met))).compile.drain
            )
          )
      case Packet(ack: Acknowledgment, met) =>
        check(met.port)(channel.send(Packet(ErrorType(NoSuchUser, "no such user is session cache"), met))) { file =>
          fs.readChunk(file, ack.block)
            .map(data => Packet(Data(ack.block, data), met))
            .compile
            .lastOrError
            .flatMap(channel.send[Data])
        }
      case Packet(err: ErrorType, met) => channel.send(Packet(err, met))
      case _                           => IO.raiseError(new RuntimeException("Internal error. Should not be!"))
    }
  }
}
