package utils

import java.io.File

/**
  * Represents various types of TFTP packet
  * @see [[https://tools.ietf.org/html/rfc1350]]
  */
sealed abstract class Packet(opcode: Int) extends Product with Serializable

sealed trait Mode { val value: String }
object Netascii extends Mode { val value: String = "netascii" }
object Octer    extends Mode { val value: String = "octet"    }
object Mail     extends Mode { val value: String = "mode"     }

/*
WARNING! after filename and mode should be 1 byte with zero value
 */
sealed abstract class IOPacket(opcode: Int, fileName: File, mode: Mode) extends Packet(opcode)
case class RRQ(fileName: File, mode: Mode)                              extends IOPacket(1, fileName, mode)
case class WRQ(fileName: File, mode: Mode)                              extends IOPacket(2, fileName, mode)

case class Block(number: Int)
case class Data(block: Block, data: String) extends Packet(3)
case class Acknowledgment(block: Block)     extends Packet(4)

trait ErrorCode {
  val value: Int
  val info: String
}
object NotDefined extends ErrorCode {
  override val value: Int   = 0
  override val info: String = "Not defined, see error message (if any)"
}
object FileNotFound extends ErrorCode {
  override val value: Int   = 1
  override val info: String = "File not found"
}
object AccessViolation extends ErrorCode {
  override val value: Int   = 2
  override val info: String = "Access violation"
}
object NotEnoughSpace extends ErrorCode {
  override val value: Int   = 3
  override val info: String = "Disk full or allocation exceeded"
}
object IllegalOperation extends ErrorCode {
  override val value: Int   = 4
  override val info: String = "Illegal TFTP operation"
}
object UnknownTID extends ErrorCode {
  override val value: Int   = 5
  override val info: String = "Unknown transfer ID"
}
object AlreadyExists extends ErrorCode {
  override val value: Int   = 6
  override val info: String = "File already exists"
}
object NoSuchUder extends ErrorCode {
  override val value: Int   = 7
  override val info: String = "No such user"
}

/*
WARNING! after errorMsg should be 1 byte with zero value
 */
case class Error(errorCode: ErrorCode, errorMsg: String) extends Packet(5)
