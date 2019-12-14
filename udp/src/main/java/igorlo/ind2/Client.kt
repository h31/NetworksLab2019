package igorlo.ind2

import igorlo.TextColors
import igorlo.ind2.Constants.DEFAULT_LOGGING_LEVEL
import igorlo.ind2.Constants.PORT
import igorlo.util.TcpExchange.readMessage
import igorlo.util.TcpExchange.sendMessage
import igorlo.util.Utilities.colorPrint
import org.apache.log4j.BasicConfigurator
import java.net.*
import java.util.*
import org.apache.log4j.Logger

class Client {
    private val logger: Logger
    private val socket: Socket
    private val readingThread: Thread

    companion object {
        private const val INFO = "\tЭто клиентское приложение для индивидуального задания\n" +
                "\tпо курсу \"Основы компьютерных сетей\".\n\n" +
                "\tАвтор - Игорь Лопатинский"
    }

    init {
        Thread.currentThread().name = "Main-client"
        BasicConfigurator.configure()
        logger = Logger.getLogger(Client::class.java)
        logger.level = DEFAULT_LOGGING_LEVEL
        logger.info("Инициализация клиента")
        socket = Socket("localhost", PORT)
        logger.info("Сокет инициализирован")
        readingThread = Thread(Runnable {
            while (true) {
                colorPrint("${readMessage(socket)}\n", TextColors.ANSI_YELLOW)
            }
        }, "Reader-client")
    }

    fun run() {
        logger.info("Начало работы клиента")
        readingThread.start()
        logger.info("Запустил отдельный поток на чтение сообщений")
        val scanner = Scanner(System.`in`)
        logger.info("Сканер консоли инициализирован")
        var command: String
        loop@ while (true) {
            Thread.sleep(100)
            colorPrint("\nВведите команду: \n", TextColors.ANSI_CYAN)
            command = scanner.nextLine()
            when (parseInput(command)) {
                Action.TO_SERVER -> {
                    sendMessage(socket, command)
                }
                Action.INFO -> {
                    printInfo()
                }
                Action.EXIT -> {
                    colorPrint("До свидания!", TextColors.ANSI_CYAN)
                    break@loop
                }
            }

        }
        close()
    }

    private fun printInfo() {
        colorPrint("\n$INFO\n", TextColors.ANSI_PURPLE)
    }

    private fun parseInput(command: String): Action {
        when (command.toLowerCase()) {
            "info" -> return Action.INFO
            "exit" -> return Action.EXIT
            else -> return Action.TO_SERVER
        }
    }

    private fun close() {
        logger.info("Закрываю сокет")
        socket.close()
        logger.info("Завершаю работу")
    }
}

data class Command(val mnemonic: String, val description: String)

private enum class Action {
    TO_SERVER, EXIT, INFO
}
