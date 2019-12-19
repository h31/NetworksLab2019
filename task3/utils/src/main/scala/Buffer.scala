import java.nio.ByteBuffer
import java.nio.charset.StandardCharsets.UTF_8

/**
  * WARNING! This API is not referential transperent and safe
  *
  * Buffer and Ops to extract bytes in a side-effectful manner.
  * Does not perform actions to check tftp-rules
  */
class Buffer private (bf: ByteBuffer) {
  def take(n: Int): Array[Byte] = {
    val res = ByteBuffer.allocate(n)
    bf.get(res.array, 0, n)
    res.array
  }

  def takeString: String = {
    val pos = bf.position()
    val res = bf
      .array()
      .slice(pos, bf.capacity() + 1)
    new String(res, UTF_8)
  }

  def takeUtnilZero: String = {
    val pos = bf.position()
    val res = bf.array
      .slice(pos, bf.capacity + 1)
      .takeWhile(_ != 0)
    bf.position(pos + res.length + 1)
    new String(res, UTF_8)
  }

  def shift(): Unit = bf.position(bf.position() + 1)

  def takeNumber: Short = bf.getShort()

  def putNumber(opcode: Short): Unit    = bf.putShort(opcode)
  def putData(array: Array[Byte]): Unit = bf.put(array)
  def takeData: Array[Byte] =
    bf.array().slice(bf.position(), bf.capacity() + 1)

  def putString(message: String): Unit = bf.put(message.getBytes(UTF_8))
  def putZero: Unit                    = bf.put(0: Byte)

  def trim(): Array[Byte] = {
    val array = bf.array().slice(bf.position(), bf.capacity() + 1)
    val zeros = array.reverse.takeWhile(_ == 0).length
    array.slice(0, array.length - zeros)
  }

  def array = bf.array
  def size  = bf.capacity()
  def pos   = bf.position()
}

object Buffer {
  def apply(capacity: Int)             = new Buffer(ByteBuffer.allocate(capacity))
  def wrap(array: Array[Byte]): Buffer = new Buffer(ByteBuffer.wrap(array))

  // wrap buffer with position shift
  def wrapUnsafe(array: Array[Byte]): Buffer = new Buffer(unsafeShifted(array))
  private[this] def unsafeShifted(array: Array[Byte]): ByteBuffer = {
    val bf = ByteBuffer.wrap(array)
    bf.position(2)
    bf
  }
}
