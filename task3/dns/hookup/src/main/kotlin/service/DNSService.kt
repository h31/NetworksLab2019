package service

import domain.model.DNSName
import domain.model.DNSResult
import domain.model.enums.DNSQueryType

interface DNSService {

    fun makeRequest(type: DNSQueryType, dnsServerAddress: String, dnsName: DNSName): DNSResult

}
