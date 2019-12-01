package domain.model.enums

enum class DNSMessageType(val value: Boolean) {

    QUERY(false),
    ANSWER(true);

    companion object {
        fun of(value: Boolean) = if (value) QUERY else ANSWER
    }

}
