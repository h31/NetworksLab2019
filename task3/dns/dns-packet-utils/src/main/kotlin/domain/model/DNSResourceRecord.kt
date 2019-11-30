package domain.model

import domain.model.enums.DNSKlass
import domain.model.enums.DNSQueryType

/**
 * Maximal size of byteArray
 * dataLength equals data.size
 */
data class DNSResourceRecord(
        val name: DNSName,
        val type: DNSQueryType,
        val klass: DNSKlass,
        val ttl: Int,
        val dataLength: Short,
        val data: DNSRRData
)
