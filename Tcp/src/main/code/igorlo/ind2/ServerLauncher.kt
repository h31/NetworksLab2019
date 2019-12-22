package igorlo.ind2

import igorlo.ind2.server.Server
import igorlo.util.TcpExchange
import org.apache.log4j.Level

object ServerLauncher {
    @JvmStatic
    fun main(args: Array<String>) {
        TcpExchange.setLoggingLevel(Level.WARN)
        Server().run()
    }
}