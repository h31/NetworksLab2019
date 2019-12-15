import java.io.File
import java.nio.charset.StandardCharsets
import java.nio.file.{Path, StandardOpenOption}

import cats.effect.{Blocker, ContextShift, IO}
import StandardOpenOption._

import cats.syntax.flatMap._
import Range.chunkSize
import fs2.io._
import fs2._
import org.slf4j.Logger

case class Range(from: Int = 0, to: Int = chunkSize)

object Range {
  val chunkSize = 512

  def other(block: Block): Range =
    Range((block.number - 1) * chunkSize, to = block.number * chunkSize)
}

case class FileSystem private (private val blocker: Blocker)(implicit val ctx: ContextShift[IO], logger: Logger) {

  /**
    * deterministic function, that will create a directory if it does not exist
    */
  def createDirectory(dir: File = FileSystem.defaultDir): IO[Path] =
    exists(dir).flatMap { _.fold(file.createDirectory[IO](blocker, dir.toPath))(_ => IO.pure(dir.toPath)) }

  def exists(path: File): IO[Option[Unit]] =
    file.exists[IO](blocker, path.toPath).map {
      case true  => Some(())
      case false => None
    }

  def readChunk(path: File, block: Block): Stream[IO, String] = {
    val range = Range.other(block)

    file
      .readRange[IO](FileSystem.otherPath(path).toPath, blocker, chunkSize, range.from, range.to)
      .through(text.utf8Decode[IO])
  }

  def writeChunk(path: File, data: String): Stream[IO, Unit] =
    Stream
      .emits(data.getBytes(StandardCharsets.UTF_8).toSeq)
      .through(
          file.writeAll[IO](
            FileSystem.otherPath(path).toPath
          , blocker
          , Seq(CREATE, APPEND)
        )
      )
}

object FileSystem {
  import FileImplicitUtils._
  val defaultDir = new File("./files")

  def otherPath(p: File): File = defaultDir / p.getName

  def apply(blocker: Blocker)(implicit ctx: ContextShift[IO], logger: Logger): IO[FileSystem] =
    IO(new FileSystem(blocker)).flatTap(_.createDirectory())
}

object FileImplicitUtils {
  implicit class RichFile(file: File) extends File(file.getName) {
    def /(other: String): File = new File(file.getAbsolutePath + s"/$other")
  }
}
