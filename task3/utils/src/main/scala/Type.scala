import java.io.File
import java.net.InetAddress
import FileImplicitUtils._
import cats.syntax.either._
import FileSystem._

import Codec.{Decoded, Decoder, Encoder}

/**
  * Represents various types of TFTP packet
  * @see [[https://tools.ietf.org/html/rfc1350]]
  */
sealed abstract class Type extends Product with Serializable { protected val opcode: Short }
case class Metadata(addr: InetAddress = InetAddress.getLocalHost, port: Int = 69)

case class Packet[T <: Type](packet: T, metadata: Metadata)

sealed trait Mode { val value: String }
object Netascii extends Mode { val value: String = "netascii" }
object Octet    extends Mode { val value: String = "octet"    }
object Mail     extends Mode { val value: String = "mode"     }

object Mode {
  val modes = List(Netascii, Octet, Mail)
}

sealed abstract class IOType(val file: File, val mode: Mode) extends Type
object IOType {
  /* WRQ and RRQ have the same rules for encoding and decoding */
  def encode[P <: IOType](packet: P): Array[Byte] = {
    val bf = Buffer(2 + packet.file.getName.length + 1 + packet.mode.value.length + 1)
    bf.putNumber(packet.opcode)
    bf.putString(packet.file.getName)
    bf.putZero
    bf.putString(packet.mode.value)
    bf.putZero

    bf.array
  }

  def decode[P <: IOType](bytes: Array[Byte])(fn: ((File, Mode)) => P): Decoded[P] = {
    val bf       = Buffer.wrapUnsafe(bytes)
    val fileName = defaultDir / bf.takeString
    val mode     = bf.takeString

    Mode.modes
      .find(_.value == mode)
      .map(fileName -> _) match {
      case Some(value) => fn(value).asRight
      case None        => new RuntimeException("there is no such mode").asLeft
    }
  }
}
case class RRQ(override val file: File, override val mode: Mode = Netascii) extends IOType(file, mode) {
  val opcode = 1
}
object RRQ {
  implicit val encoder: Encoder[RRQ] = (msg: RRQ) => IOType.encode(msg)
  implicit val decoder: Decoder[RRQ] = new Decoder[RRQ] {
    override def decode[U >: RRQ](bytes: Array[Byte]): Decoded[U] =
      IOType.decode[RRQ](bytes) {
        case (file, mode) => RRQ(file, mode)
      }
  }
}

case class WRQ(override val file: File, override val mode: Mode = Netascii) extends IOType(file, mode) {
  val opcode = 2
}
object WRQ {
  implicit val encoder: Encoder[WRQ] = (msg: WRQ) => IOType.encode(msg)
  implicit val decoder: Decoder[WRQ] =
    new Decoder[WRQ] {
      override def decode[U >: WRQ](bytes: Array[Byte]): Decoded[U] =
        IOType.decode[WRQ](bytes) {
          case (file, mode) => WRQ(file, mode)
        }
    }
}

case class Block(number: Short)
case class Data(block: Block, data: String) extends Type { val opcode = 3 }
object Data {
  implicit val encoder: Encoder[Data] = (msg: Data) => {
    val bf = Buffer(2 + 2 + msg.data.length)
    bf.putNumber(msg.opcode)
    bf.putNumber(msg.block.number)
    bf.putString(msg.data)

    bf.array
  }
  implicit val decoder: Decoder[Data] = new Decoder[Data] {
    override def decode[U >: Data](bytes: Array[Byte]): Decoded[U] = {
      val bf = Buffer.wrapUnsafe(bytes)
      Data(Block(bf.takeNumber), bf.takeString).asRight
    }
  }
}
case class Acknowledgment(block: Block) extends Type { val opcode = 4 }
object Acknowledgment {
  implicit val encoder: Encoder[Acknowledgment] = (ack: Acknowledgment) => {
    val bf = Buffer(2 + 2)
    bf.putNumber(ack.opcode)
    bf.putNumber(ack.block.number)

    bf.array
  }
  implicit val decoder: Decoder[Acknowledgment] = new Decoder[Acknowledgment] {
    override def decode[U >: Acknowledgment](bytes: Array[Byte]): Decoded[U] =
      Acknowledgment(Block(Buffer.wrapUnsafe(bytes).takeNumber)).asRight
  }
}

trait ErrorCode {
  val value: Short
  val info: String
}
object NotDefined extends ErrorCode {
  val value = 0
  val info  = "Not defined, see error message (if any)"
}
object FileNotFound extends ErrorCode {
  val value = 1
  val info  = "File not found"
}
object AccessViolation extends ErrorCode {
  val value = 2
  val info  = "Access violation"
}
object NotEnoughSpace extends ErrorCode {
  val value = 3
  val info  = "Disk full or allocation exceeded"
}
object IllegalOperation extends ErrorCode {
  val value = 4
  val info  = "Illegal TFTP operation"
}
object UnknownTID extends ErrorCode {
  val value = 5
  val info  = "Unknown transfer ID"
}
object AlreadyExists extends ErrorCode {
  val value = 6
  val info  = "File already exists"
}
object NoSuchUser extends ErrorCode {
  val value = 7
  val info  = "No such user"
}

/*
WARNING! after errorMsg should be 1 byte with zero value
 */
case class ErrorType(errorCode: ErrorCode, errorMsg: String) extends Type { val opcode = 5 }
object ErrorType {
  implicit val encoder: Encoder[ErrorType] = (msg: ErrorType) => {
    val bf = Buffer(2 + 2 + msg.errorMsg.length + 1)
    bf.putNumber(msg.opcode)
    bf.putNumber(msg.errorCode.value)
    bf.putString(msg.errorMsg)
    bf.putZero

    bf.array
  }
  implicit val decoder: Decoder[ErrorType] = new Decoder[ErrorType] {
    override def decode[U >: ErrorType](bytes: Array[Byte]): Decoded[U] = {
      val bf     = Buffer.wrapUnsafe(bytes)
      val number = bf.takeNumber
      errors
        .find(_.value == number)
        .map { errorCode =>
          ErrorType(errorCode, bf.takeString)
        } match {
        case Some(v) => v.asRight
        case None    => new RuntimeException("There is no such value").asLeft
      }
    }
  }

  val errors = List(
      NotDefined
    , FileNotFound
    , AccessViolation
    , NotEnoughSpace
    , IllegalOperation
    , UnknownTID
    , AlreadyExists
    , NoSuchUser
  )
}
