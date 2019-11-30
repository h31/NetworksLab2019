package extensions

fun Int.lowestShort() = toShort()

fun Int.highestShort() = ushr(Short.SIZE_BITS).toShort()