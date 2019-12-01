package utils

import domain.model.DNSPacket
import domain.model.DNSRRData
import domain.model.DNSResourceRecord
import domain.model.enums.DNSRCode

object DNSPacketPrinter {

    private const val internalErrorMessage = "Произошла внутренняя ошибка сервера"
    private const val nameErrorMessage = "Отсутствует запись для заданного доменного имени"
    private const val unsupportedQueryMessage = "Заданная запрос не поддерживается DNS - сервером"
    private const val undefinedErrorCodeMessage = "Получен неизвестный код результата: %s"

    fun printPacket(packet: DNSPacket) {
        println(when (packet.header.flags.rCode) {
            DNSRCode.NO_ERROR -> printNoError(packet)
            DNSRCode.INTERNAL_ERROR -> internalErrorMessage
            DNSRCode.NAME_ERROR -> nameErrorMessage
            DNSRCode.UNSUPPORTED -> unsupportedQueryMessage
            DNSRCode.UNDEFINED -> String.format(undefinedErrorCodeMessage, packet.header.flags.rCode.value.toString(2))
        })
    }

    private fun printNoError(packet: DNSPacket) = packet.answers.joinToString("\n", transform = ::getDNSResourceRecordString)

    private fun getDNSResourceRecordString(rr: DNSResourceRecord) = "type: ${rr.type}\n\t" + when (rr.data) {
        is DNSRRData.A -> "ip: ${IP.intIPv4toString((rr.data as DNSRRData.A).address)}"
        is DNSRRData.CName -> (rr.data as DNSRRData.CName).name
        is DNSRRData.NS -> (rr.data as DNSRRData.NS).name
        is DNSRRData.MX -> (rr.data as DNSRRData.MX).let { "${it.preference}\t${it.exchange}" }
        is DNSRRData.HInfo -> (rr.data as DNSRRData.HInfo).cpuAndOs
        DNSRRData.Undefined -> ""
    }

}
