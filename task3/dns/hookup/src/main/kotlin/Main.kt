import args4j.ArgumentsParser
import domain.model.DNSResult
import service.DNSService
import service.impl.DNSServiceImpl
import utils.DNSPacketPrinter

fun main(args: Array<String>) {
    val parsedArgs = ArgumentsParser.parse(args)
    val service: DNSService = DNSServiceImpl()
    when (val result = service.makeRequest(parsedArgs.type, parsedArgs.serverAddress, parsedArgs.name)) {
        is DNSResult.Data -> DNSPacketPrinter.printPacket(result.packet)
        is DNSResult.Error -> System.err.println(result.cause)
    }
}
