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
        is DNSRRData.CName,
        is DNSRRData.NS,
        is DNSRRData.MX,
        is DNSRRData.HInfo -> getNameString(rr.data)
        DNSRRData.Undefined -> ""
    }

    private fun getNameString(rr: DNSRRData): String = when (rr) {
        is DNSRRData.CName -> rr.name.toString()
        is DNSRRData.NS -> rr.name.toString()
        is DNSRRData.MX -> "$rr"
        is DNSRRData.HInfo -> rr.cpuAndOs.marks.joinToString(" ")
        else -> ""
    }

}
