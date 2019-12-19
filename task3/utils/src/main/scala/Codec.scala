object Codec {
  type Msg        = Type
  type Decoded[T] = Either[Throwable, T]

  trait Encoder[-Msg] {
    def encode(msg: Msg): Array[Byte]
  }

  trait Decoder[+Msg] {
    def decode[U >: Msg](bytes: Array[Byte]): Decoded[U]
  }

  object Decoder {
    def decode[T <: Msg](bytes: Array[Byte])(implicit d: Decoder[T]): Decoded[T] = d.decode(bytes)
  }
}
