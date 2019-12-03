package igorlo.ind2.server

import igorlo.TextColors
import igorlo.ind2.Constants.CHUNK_SIZE
import igorlo.ind2.Constants.COMMANDS
import igorlo.ind2.Constants.CONSOLE_WIDTH
import igorlo.util.Utilities.SUPPORTED_OPERATIONS
import igorlo.util.Utilities.calculateFactorial
import igorlo.util.Utilities.calculateSqrt
import igorlo.util.Utilities.colorPrint
import java.net.Socket

import igorlo.util.Exchange.sendMessage as send

object Handlers {

    fun handleOperators(clientSocket: Socket) {
        val stringBuilder = StringBuilder()
        stringBuilder.append("Операторы, доступные в рамках комманды eval:\n")
        for (operator in SUPPORTED_OPERATIONS){
            stringBuilder.append("$operator\n")
        }
        send(clientSocket, stringBuilder.toString())
    }

    fun handleHistory(clientSocket: Socket) {
        //TODO
    }

    fun handleCommands(clientSocket: Socket) {
        val stringBuilder = StringBuilder()
        stringBuilder.append("Список доступных команд:\n")
        for (command in COMMANDS){
            stringBuilder.append("${command.mnemonic.padEnd(CONSOLE_WIDTH/3)}| ${command.description}\n")
        }
        send(clientSocket, stringBuilder.toString())
    }

    fun handleUnknown(clientSocket: Socket) {
        send(clientSocket, "Неизвестная команда")
    }

    fun handleSay(clientSocket: Socket, message: String) {
        colorPrint("Сообщение от клиента : $message\n", TextColors.ANSI_PURPLE)//TODO добавить имя
        send(clientSocket, "Ваше сообщение доставлено.")
    }

    fun handleSqrt(clientSocket: Socket, sqrt: String) {
        Thread(Runnable {
            val response = calculateSqrt(sqrt)
            send(clientSocket, response)
        }, "Factorial-worker").start()
    }

    fun handleFact(clientSocket: Socket, factorial: String) {
        Thread(Runnable {
            val response = calculateFactorial(factorial)
            send(clientSocket, "Факториал числа $factorial:")
            if (response.length < CHUNK_SIZE) {
                send(clientSocket, response)
            } else {
                var chunked = response
                while (chunked.isNotEmpty()) {
                    val chunk = chunked.take(CHUNK_SIZE)
                    send(clientSocket, chunk)
                    chunked = chunked.removePrefix(chunk)
                }
            }
        }, "Factorial-worker").start()
    }

}