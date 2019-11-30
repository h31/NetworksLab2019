package domain.model

import domain.model.enums.DNSMessageType
import domain.model.enums.DNSOpCode
import domain.model.enums.DNSRCode

data class DNSFlags(
        val qr: DNSMessageType, // 0 - запрос, 1 - ответ
        val opCode: DNSOpCode, // 0 - стандартный запрос, ...
        val aa: Boolean, // авторитетный ответ
        val tc: Boolean, // усечение (TrunCation) указывает на усечение сообщения
        val rd: Boolean, // желательная рекурсия
        val ra: Boolean, // рекурсия доступна
        val rCode: DNSRCode // код ответа
)
