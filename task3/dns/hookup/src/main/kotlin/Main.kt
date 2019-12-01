import args4j.ArgumentsParser
import domain.model.DNSResult
import service.DNSService
import service.impl.DNSServiceImpl
import utils.DNSPacketPrinter
import kotlin.system.exitProcess

fun main(args: Array<String>) {
    val parsedArgs = ArgumentsParser.parse(args)
    val service: DNSService = DNSServiceImpl()
    service.makeRequest(parsedArgs.type, parsedArgs.serverAddress, parsedArgs.name) { result ->
        when (result) {
            is DNSResult.Data -> DNSPacketPrinter.printPacket(result.packet)
            is DNSResult.Error -> System.err.println(result.cause)
        }
        exitProcess(0)
    }
}
