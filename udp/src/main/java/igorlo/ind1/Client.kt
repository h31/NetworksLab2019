package igorlo.ind1

import igorlo.TextColors
import igorlo.util.Exchange.readMessage
import igorlo.util.Exchange.sendMessage
import igorlo.util.Utilities.colorPrint
import org.apache.log4j.BasicConfigurator
import java.net.*
import java.util.*
import org.apache.log4j.Logger
import igorlo.util.Utilities.Command
import java.lang.NumberFormatException

class Client {
    private val logger: Logger
    private val socket: Socket
    private val readingThread: Thread

    companion object {
        private const val PORT = 8888
        private const val CONSOLE_WIDTH = 100
        private const val WAIT_TIME = 200L
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
        socket = Socket("localhost", PORT)
        logger.info("Сокет инициализирован")
        readingThread = Thread(Runnable {
            while (true) {
                colorPrint("\n${readMessage(socket)}\n", TextColors.ANSI_YELLOW)
            }
        }, "Reader")
    }

    private fun validate(command: String): Boolean {
        if (command.isEmpty()) {
            validationError("Пустая команда")
            return false
        }
        val sliced: List<String> = command.split(' ')
        when (sliced[0]) {
            "a" -> {
                if (sliced.size != 5) {
                    validationError("Неверное число аргументов")
                    return false
                }
                if (!validateInt(sliced[1])) {
                    validationError("Второй аргумент не является целым числом")
                    return false
                }
                if (!validateDouble(sliced[4])) {
                    validationError("Четвёртый аргумент не является числом")
                    return false
                }
                return true
            }
            "d" -> {
                if (sliced.size != 2) {
                    validationError("Неверное число аргументов")
                    return false
                }
                if (!validateInt(sliced[1])) {
                    validationError("Второй аргумент не является целым числом")
                    return false
                }
                return true
            }
            "r" -> {
                if (sliced.size != 1) {
                    validationError("Неверное число аргументов")
                    return false
                }
                return true
            }
            "c" -> {
                if (sliced.size != 3) {
                    validationError("Неверное число аргументов")
                    return false
                }
                if (!validateInt(sliced[1])) {
                    validationError("Второй аргумент не является целым числом")
                    return false
                }
                if (!validateDouble(sliced[2])) {
                    validationError("Четвёртый аргумент не является числом")
                    return false
                }
                return true
            }
            "h" -> {
                if (sliced.size != 2) {
                    validationError("Неверное число аргументов")
                    return false
                }
                if (!validateInt(sliced[1])) {
                    validationError("Второй аргумент не является целым числом")
                    return false
                }
                return true
            }
            else -> {
                validationError("Не удалось распознать комманду")
                return false
            }
        }
    }

    private fun validateDouble(possibleDouble: String): Boolean {
        try {
            possibleDouble.toDouble()
            return true
        } catch (e: NumberFormatException) {
            return false
        }
    }

    private fun validateInt(possibleInt: String): Boolean {
        try {
            possibleInt.toInt()
            return true
        } catch (e: NumberFormatException) {
            return false
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
        colorPrint("Ошибка валидации\n$error", TextColors.ANSI_RED)
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
    }
}

enum class Action {
    TO_SERVER, HELP, EXIT, INFO
}
