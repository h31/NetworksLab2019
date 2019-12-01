import koin.appModule
import org.koin.core.context.startKoin
import services.Server
import utils.inject

fun main() {
    startKoin {
        printLogger()
        fileProperties()
        modules(appModule)
    }
    val server by inject<Server>()
    server.start()
}
