package domain.model

class DNSName private constructor(val marks: List<String>) {

    class Builder {

        companion object {
            private const val MAX_MARK_SIZE = 63
            // Max name list size is 127 (The maximal depth of a domain tree)
            private const val MAX_MARKS_NUM = 127
        }

        private val marks: MutableList<String> = mutableListOf()

        fun addMark(mark: String) {
            checkMarks(mark)
            marks += mark
        }

        fun build() = DNSName(marks)

        private fun checkMarks(mark: String) {
            check(mark.length <= MAX_MARK_SIZE) {
                "Too big mark size. Maximal mark size is $MAX_MARK_SIZE, but got ${mark.length}"
            }
            check(marks.size <= MAX_MARKS_NUM) {
                "Too many marks. Maximal number of marks is $MAX_MARKS_NUM"
            }
        }

    }

    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as DNSName

        if (marks == other.marks) return true

        return false
    }

    override fun hashCode() = marks.hashCode()

    override fun toString() = marks.joinToString(separator = ".")

}
