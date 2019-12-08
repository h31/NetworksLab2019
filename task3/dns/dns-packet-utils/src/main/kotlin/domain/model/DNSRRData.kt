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
            val name: DNSName
    ) : DNSRRData() {

        override fun getDataLength() = name.getSize()

    }

    data class HInfo(
            val cpuAndOs: DNSName
    ) : DNSRRData() {

        override fun getDataLength() = cpuAndOs.getSize()

    }

    data class MX(
            val preference: Short,
            val exchange: DNSName
    ) : DNSRRData() {

        override fun getDataLength() = (Short.SIZE_BYTES + exchange.getSize()).toShort()

    }

    data class NS(
            val name: DNSName
    ) : DNSRRData() {

        override fun getDataLength() = name.getSize()

    }

    object Undefined : DNSRRData() {
        override fun getDataLength() = 0.toShort()
    }

}
