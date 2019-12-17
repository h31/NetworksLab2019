import java.io.File
import java.nio.charset.StandardCharsets

import cats.effect.IO
import fs2._
import cats.syntax.flatMap._

object Rpc {

  private[this] val haltCriteria: Int => Boolean = size => size < Range.chunkSize

  def upload(path: File, fileName: File)(channel: SafeUdpChannel, fs: FileSystem): IO[Unit] = {
    def validate(p: Packet[Type]): IO[Unit] =
      p match {
        case Packet(Data(Block(number), data), metadata) =>
          (fs.writeChunk(path, data) ++ Stream.eval {
            if (haltCriteria(data.getBytes(StandardCharsets.UTF_8).length)) IO.unit
            else channel.send(Packet(Acknowledgment(Block((number + 1).toShort)), metadata)) >> loop
          }).compile.drain
        case Packet(ErrorType(err, msg), _) =>
          IO.raiseError(new RuntimeException(s"error from server: ${err.info}, $msg"))
        case _ => IO.raiseError(new RuntimeException("Wrong type of package"))
      }

    def loop: IO[Unit] =
      for {
        packet <- channel.receive
        res    <- validate(packet)
      } yield res

    channel.send(Packet(RRQ(fileName), Metadata())) >> loop
  }

  def download(file: File)(channel: SafeUdpChannel, fs: FileSystem): IO[Unit] = {
    def validate(p: Packet[Type]): IO[Unit] =
      p match {
        case Packet(ErrorType(err, msg), _) =>
          IO.raiseError(new RuntimeException(s"error from server: ${err.info}, $msg"))
        case Packet(Acknowledgment(block), _) =>
          val nextBlock = Block((block.number + 1).toShort)
          fs.readChunk(file, nextBlock)
            .flatMap { str =>
              val send = Stream.eval(channel.send(Packet(Data(nextBlock, str), Metadata())))
              val next =
                if (haltCriteria(str.getBytes(StandardCharsets.UTF_8).length)) IO.unit
                else loop

              send ++ Stream.eval(next)
            }
            .compile
            .drain
        case _ => IO.raiseError(new RuntimeException("Wrong type of package"))
      }

    def loop: IO[Unit] =
      for {
        packet <- channel.receive
        res    <- validate(packet)
      } yield res

    channel.send(Packet(WRQ(file), Metadata())) >> loop
  }

}
