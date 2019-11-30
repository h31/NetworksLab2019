package utils

import domain.model.DNSFlags
import domain.model.DNSHeader
import domain.model.DNSName
import domain.model.DNSPacket
import domain.model.DNSQuery
import domain.model.enums.DNSKlass
import domain.model.enums.DNSMessageType
import domain.model.enums.DNSOpCode
import domain.model.enums.DNSQueryType
import domain.model.enums.DNSRCode

object DNSQueryPacketBuilder {

    fun buildDNSQueryPacket(id: Short, type: DNSQueryType, name: DNSName) = DNSPacket(
            header = DNSHeader(
                    id = id,
                    flags = DNSFlags(
                            qr = DNSMessageType.QUERY,
                            opCode = DNSOpCode.STANDARD,
                            aa = false,
                            tc = false,
                            rd = false,
                            ra = false,
                            rCode = DNSRCode.NO_ERROR
                    ),
                    numOfQueries = 1,
                    numOfAnswers = 0,
                    numOfAdditionalAnswers = 0,
                    numOfAuthorityAnswers = 0
            ),
            queries = listOf(DNSQuery(name = name, type = type, klass = DNSKlass.IN)),
            answers = emptyList(),
            additionalAnswers = emptyList(),
            authorityAnswers = emptyList()
    )

}
