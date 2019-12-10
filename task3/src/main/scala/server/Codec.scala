package server

import utils.Packet

object Codec {
  type Msg = Packet

  trait Encoded[-Msg] {
    def encode(msg: Msg): Array[Byte]
  }

  trait Decoded[+Msg] {
    def decode(bytes: Array[Byte]): Msg
  }
}
