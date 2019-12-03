package igorlo.ind2

import igorlo.ind2.server.Server
import igorlo.util.Exchange
import org.apache.log4j.Level

object Main {

    @JvmStatic
    fun main(args: Array<String>) {
        Exchange.setLoggingLevel(Level.WARN)
        Thread(Runnable {
            Server().run()
        }).start()
        Thread.sleep(500)
        Thread(Runnable {
            Client().run()
        }).start()
    }

}