package domain.model

import domain.model.enums.DNSKlass
import domain.model.enums.DNSQueryType

data class DNSQuery(
        val name: DNSName,
        val type: DNSQueryType,
        val klass: DNSKlass
)
