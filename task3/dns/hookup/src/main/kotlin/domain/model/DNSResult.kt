package domain.model

sealed class DNSResult {

    class Data(val packet: DNSPacket) : DNSResult()

    class Error(val cause: String) : DNSResult()

}
