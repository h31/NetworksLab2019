package domain.model.enums

enum class DNSRCode(val value: Byte) {

    NO_ERROR(0),
    INTERNAL_ERROR(2), // Внутренняя ошибка сервера
    NAME_ERROR(3), // Такое доменное имя не существует
    UNSUPPORTED(4), // Нет реализации для данного вида запроса
    UNDEFINED(Byte.MAX_VALUE);

    companion object {
        fun of(value: Byte) = values().find { it.value == value } ?: UNDEFINED
    }

}
