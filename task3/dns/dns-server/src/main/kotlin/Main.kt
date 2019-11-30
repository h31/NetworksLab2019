import koin.myModule
import org.koin.core.context.startKoin
import services.Server
import utils.inject

fun main() {
    startKoin {
        printLogger()
        fileProperties()
        modules(myModule)
    }
    val server by inject<Server>()
    server.start()
}
