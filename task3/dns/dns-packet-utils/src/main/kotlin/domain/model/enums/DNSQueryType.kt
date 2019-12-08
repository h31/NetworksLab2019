package domain.model.enums

enum class DNSQueryType(val value: Short) {

    A(1),
    NS(2),
    CNAME(5),
    H_INFO(13),
    MX(15),
    UNDEFINED(Short.MAX_VALUE);

    companion object {
        fun of(value: Short) = values().find { it.value == value } ?: UNDEFINED
    }

}
