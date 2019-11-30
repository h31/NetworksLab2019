package domain.model

data class DNSHeader(
        val id: Short, // уникальный ID, одинаков в запросе и ответе
        val flags: DNSFlags,
        val numOfQueries: Short,
        val numOfAnswers: Short,
        val numOfAuthorityAnswers: Short,
        val numOfAdditionalAnswers: Short
)
