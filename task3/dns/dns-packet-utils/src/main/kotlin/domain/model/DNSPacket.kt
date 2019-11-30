package domain.model

data class DNSPacket(
        val header: DNSHeader,
        val queries: List<DNSQuery>,
        val answers: List<DNSResourceRecord>,
        val authorityAnswers: List<DNSResourceRecord>,
        val additionalAnswers: List<DNSResourceRecord>
)
