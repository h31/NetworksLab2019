import koin.myModule
import org.koin.core.context.startKoin
import utils.DNSPacketCompressor

// nslookup example.com. localhost

fun main() {
    startKoin {
        printLogger()
        fileProperties()
        modules(myModule)
    }
    Server().start()
}
