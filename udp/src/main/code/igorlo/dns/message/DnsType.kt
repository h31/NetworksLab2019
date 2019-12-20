package igorlo.dns.message

enum class DnsType (val type: Int, val desc: String) {
    A(1, "Address"),
    NS(2, "Authoritative name server")
}