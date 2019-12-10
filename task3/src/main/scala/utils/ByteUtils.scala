package utils

object ByteUtils {
  def opcode(value: Int): Array[Byte] = Array(value.toByte)
}
