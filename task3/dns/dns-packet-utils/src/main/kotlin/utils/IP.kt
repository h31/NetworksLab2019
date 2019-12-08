package utils

object IP {

    private const val mask: Int = 0xFF
    private const val ipAddressForamtErrorMessage = "Ip address has to math the following format: n.n.n.n where 'n' is a number in range 0..255"

    @Throws(exceptionClasses = [NumberFormatException::class, IllegalArgumentException::class])
    fun stringIPv4ToInt(ip: String) = ip
            .split(".")
            .reversed()
            .map(String::toInt)
            .check()
            .reduceIndexed { index, acc, i ->
                acc.or(i.shl(index * Byte.SIZE_BITS))
            }

    fun intIPv4toString(ip: Int) = List(Int.SIZE_BYTES) { index -> ip.ushr(index * Byte.SIZE_BITS).and(mask) }
            .reversed()
            .joinToString(separator = ".")

    @Throws(exceptionClasses = [IllegalArgumentException::class])
    private fun List<Int>.check(): List<Int> = apply {
        require(size == Int.SIZE_BYTES) { ipAddressForamtErrorMessage }
        require(any { it in 1..255 }) { ipAddressForamtErrorMessage }
    }

}
