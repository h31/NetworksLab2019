package domain.model

/**
 * Resource record data
 */
sealed class DNSRRData {

    abstract fun getDataLength(): Short

    data class A(
            val address: Int
    ) : DNSRRData() {

        override fun getDataLength() = Int.SIZE_BYTES.toShort()

    }

    data class CName(
            val name: String
    ) : DNSRRData() {

        override fun getDataLength() = name.length.toShort()

    }

    data class HInfo(
            val cpuAndOs: String
    ) : DNSRRData() {

        override fun getDataLength() = cpuAndOs.length.toShort()

    }

    data class MX(
            val preference: Short,
            val exchange: String
    ) : DNSRRData() {

        override fun getDataLength() = (Short.SIZE_BYTES + exchange.length + 1).toShort()

    }

    data class NS(
            val name: String
    ) : DNSRRData() {

        override fun getDataLength() = name.length.toShort()

    }

    object Undefined : DNSRRData() {
        override fun getDataLength() = 0.toShort()
    }

}
