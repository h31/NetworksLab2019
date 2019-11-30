package domain.model.enums

enum class DNSKlass(val value: Short) {

    IN(1);

    companion object {
        fun of(value: Short) = values().find { it.value == value } ?: IN
    }

}
