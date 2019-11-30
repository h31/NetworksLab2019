package domain.model.enums

enum class DNSMessageType(val value: Boolean) {

    QUERY(true),
    ANSWER(false);

    companion object {
        fun of(value: Boolean) = if (value) QUERY else ANSWER
    }

}
