package domain.model.enums

enum class DNSOpCode(val value: Byte) {

    STANDARD(0),
    UNDEFINED(Byte.MAX_VALUE);

    companion object {
        fun of(value: Byte) = values().find { it.value == value } ?: UNDEFINED
    }

}
