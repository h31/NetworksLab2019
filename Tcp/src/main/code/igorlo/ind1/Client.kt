package igorlo.ind1

import igorlo.TextColors
import igorlo.util.TcpExchange.readMessage
import igorlo.util.TcpExchange.sendMessage
import igorlo.util.Utilities.colorPrint
import org.apache.log4j.BasicConfigurator
import java.net.*
import java.util.*
import org.apache.log4j.Logger
import igorlo.util.Utilities.Command
import java.io.IOException
import java.lang.IllegalArgumentException
import java.lang.NumberFormatException
import kotlin.system.exitProcess

class Client {
    private val logger: Logger
    private val socket: Socket
    private val readingThread: Thread
    private var lastCommand: String = "x"

    companion object {
        private const val PORT = 8888
        private const val CONSOLE_WIDTH = 100
        private const val WAIT_TIME = 200L
        private const val TABLE_WIDTH = 12
        private const val INFO = "\tЭто клиентское приложение для индивидуального задания\n" +
                "\tпо курсу \"Основы компьютерных сетей\".\n\n" +
                "\tАвтор - Игорь Лопатинский"
        private val COMMAND_LIST = listOf(
                Command(
                        "A <ID> <NAME> <SHORT NAME> <COURCE>",
                        "Добавить новую валюту"
                ),
                Command(
                        "D <ID>",
                        "Удалить валюту"
                ),
                Command(
                        "R",
                        "Посмотреть текущие состояния валют"
                ),
                Command(
                        "C <ID> <COURCE>",
                        "Изменить курс валюты"
                ),
                Command(
                        "H <ID>",
                        "Посмотреть историю курса валюты"
                ),
                Command(
                        "EXIT",
                        "Выйти из программы"
                ),
                Command(
                        "HELP",
                        "Список команд"
                ),
                Command(
                        "INFO",
                        "Информация о проекте"
                )
        )
    }

    init {
        BasicConfigurator.configure()
        logger = Logger.getLogger(Client::class.java)
        logger.info("Инициализация клиента")
        try {
            socket = Socket("localhost", PORT)
        } catch (e: IOException) {
            logger.error("Не удалось подключиться к серверу")
            exitProcess(1)
        }
        logger.info("Сокет инициализирован")
        readingThread = Thread(Runnable {
            while (true) {
                handleReading()
            }
        }, "Reader")
    }

    private fun handleReading() {
        val message = readMessage(socket)
        if (message == "/goodbye") {
            close()
        }
        logger.info("Текст сообщения: $message")
        when (lastCommand[0]) {
            'h' -> printHistory(message)
            'r' -> printTable(message)
            else -> {
                colorPrint("\n${message}\n", TextColors.ANSI_YELLOW)
            }
        }
    }

    private fun printTable(message: String) {
        val lines = message.split("\n")
        //ID Country Currency Course absCource conCource
        colorPrint("ID".take(TABLE_WIDTH).padEnd(TABLE_WIDTH), TextColors.ANSI_BLUE)
        colorPrint("Country".take(TABLE_WIDTH).padEnd(TABLE_WIDTH), TextColors.ANSI_BLUE)
        colorPrint("Currency".take(TABLE_WIDTH).padEnd(TABLE_WIDTH), TextColors.ANSI_BLUE)
        colorPrint("Course".take(TABLE_WIDTH).padEnd(TABLE_WIDTH), TextColors.ANSI_BLUE)
        colorPrint("absCourse".take(TABLE_WIDTH).padEnd(TABLE_WIDTH), TextColors.ANSI_BLUE)
        colorPrint("conCourse\n", TextColors.ANSI_BLUE)
        for (line in lines) {
            val parts = line.split(",")
            for (part in parts) {
                colorPrint(part.take(TABLE_WIDTH).padEnd(TABLE_WIDTH), TextColors.ANSI_YELLOW)
            }
            println()
        }
    }

    private fun printHistory(message: String) {
        if (message.isEmpty()) {
            return
        }
        val splited = lastCommand.split(" ")
        val history = message.split(",")
        colorPrint("История валюты с ID = ${splited[1]}:\n", TextColors.ANSI_YELLOW)
        for (course in history) {
            colorPrint("$course\n", TextColors.ANSI_YELLOW)
        }
    }

    private fun validate(command: String): Boolean {
        if (command.isEmpty()) {
            validationError("Пустая команда")
            return false
        }
        val sliced: List<String> = command.split(' ')
        try {
            when (sliced[0]) {
                "a" -> {
                    validateArgumentQuantity(sliced.size, 5)
                    validateInt(sliced, 1)
                    validateDouble(sliced, 4)
                }
                "d" -> {
                    validateArgumentQuantity(sliced.size, 2)
                    validateInt(sliced, 1)
                }
                "r" -> {
                    validateArgumentQuantity(sliced.size, 1)
                }
                "c" -> {
                    validateArgumentQuantity(sliced.size, 3)
                    validateInt(sliced, 1)
                    validateDouble(sliced, 2)
                }
                "h" -> {
                    validateArgumentQuantity(sliced.size, 2)
                    validateInt(sliced, 1)
                }
                else -> {
                    validationError("Не удалось распознать комманду")
                }
            }
        } catch (e: IllegalArgumentException) {
            return false
        }
        return true
    }

    private fun validateArgumentQuantity(actual: Int, expected: Int) {
        if (actual != expected)
            validationError("Данная команда должна содержать $expected аргументов")
    }

    private fun validateDouble(arguments: List<String>, index: Int) {
        try {
            val possibleDouble = arguments[index]
            possibleDouble.toDouble()
        } catch (e: NumberFormatException) {
            validationError("${index + 1} аргумент не является числом")
        }
    }

    private fun validateInt(arguments: List<String>, index: Int) {
        try {
            val possibleInt = arguments[index]
            possibleInt.toInt()
        } catch (e: NumberFormatException) {
            validationError("$index аргумент не является целочисленным")
        }
    }

    fun run() {
        logger.info("Начало работы клиента")
        readingThread.start()
        logger.info("Запустил отдельный поток на чтение сообщений")
        val scanner = Scanner(System.`in`)
        logger.info("Сканер консоли инициализирован")
        var command: String
        loop@ while (true) {
            Thread.sleep(WAIT_TIME)
            colorPrint("\nВведите команду: \n", TextColors.ANSI_CYAN)
            command = scanner.nextLine().trim().toLowerCase()
            when (parseInput(command)) {
                Action.TO_SERVER -> {
                    if (validate(command)) {
                        lastCommand = command
                        sendMessage(socket, command)
                    }
                }
                Action.HELP -> {
                    printHelp()
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

    private fun validationError(error: String) {
//        colorPrint("Ошибка валидации\n$error", TextColors.ANSI_RED)
        throw IllegalArgumentException("Ошибка валидации\n$error")
    }

    private fun printInfo() {
        colorPrint("\n$INFO\n", TextColors.ANSI_PURPLE)
    }

    private fun printHelp() {
        colorPrint("\n-".padEnd(CONSOLE_WIDTH, '-'), TextColors.ANSI_WHITE)
        colorPrint("\nСписок команд".padStart(CONSOLE_WIDTH / 2 - 6), TextColors.ANSI_WHITE)
        colorPrint("\n-".padEnd(CONSOLE_WIDTH, '-'), TextColors.ANSI_WHITE)
        for (command in COMMAND_LIST) {
            colorPrint(
                    "\n${command.mnemonic.padEnd(CONSOLE_WIDTH / 2 - 2)}| ${command.description}",
                    TextColors.ANSI_BLUE
            )
        }
        colorPrint("\n-".padEnd(CONSOLE_WIDTH, '-'), TextColors.ANSI_WHITE)
    }

    private fun parseInput(command: String): Action {
        when (command.toLowerCase()) {
            "info" -> return Action.INFO
            "exit" -> return Action.EXIT
            "help" -> return Action.HELP
            else -> return Action.TO_SERVER
        }
    }

    private fun close() {
        logger.info("Закрываю сокет")
        socket.close()
        logger.info("Завершаю работу")
        exitProcess(1)
    }
}

enum class Action {
    TO_SERVER, HELP, EXIT, INFO
}
