package igorlo.ind2

import igorlo.util.Exchange.readMessage
import igorlo.util.Exchange.sendMessage
import java.io.IOException
import java.net.ServerSocket
import java.net.Socket
import javax.script.ScriptEngineManager
import javax.script.ScriptException

class Server {

    companion object {
        const val PORT = 1488
    }

    private val acceptingSocket = ServerSocket(PORT)
    private val clients = mutableListOf<Socket>()

    init {
        Thread.currentThread().name = "Main-server"
    }

    fun run() {
        while (true) {
            try {
                val newClient = acceptingSocket.accept()
                clients.add(newClient)
                Thread(Runnable {
                    handleClient(newClient)
                }, "Reader-server").start()
            } catch (e: IOException) {
                e.printStackTrace()
                break
            }
        }
    }

    private fun handleClient(newClient: Socket) {
        while (true) {
            val message = readMessage(newClient)
            try {
                val engine = ScriptEngineManager().getEngineByExtension("js")
                val result = engine.eval(message)
                sendMessage(newClient, result.toString())
            } catch (e : ScriptException){
                sendMessage(newClient, "\"Не удалось вычислить выражение.\"")
            }
        }
    }
}