package extensions

fun Short.lowestByte() = toByte()

fun Short.highestByte() = toInt().ushr(Byte.SIZE_BITS).toByte()
