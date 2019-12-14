package igorlo.ind2.server

import igorlo.TextColors
import igorlo.ind2.Constants
import igorlo.ind2.Constants.CHUNK_SIZE
import igorlo.ind2.Constants.PORT
import igorlo.ind2.server.data.UserBase
import igorlo.util.TcpExchange.readMessage
import igorlo.util.TcpExchange.sendMessage
import igorlo.util.Utilities
import igorlo.util.Utilities.evalExpression
import java.io.IOException
import java.net.ServerSocket
import java.net.Socket
import java.util.concurrent.ConcurrentHashMap

class Server {

    private val acceptingSocket: ServerSocket = ServerSocket(PORT)
    private val userBase = UserBase()
    private val activeUsers: MutableMap<Socket, String> = ConcurrentHashMap()
    private val histories: MutableMap<String, MutableList<String>> = ConcurrentHashMap()

    init {
        Thread.currentThread().name = "Main-server"
    }

    fun run() {
        while (true) {
            try {
                val newClient = acceptingSocket.accept()
                Thread(Runnable {
                    handleClient(newClient)
                }, "Handler-server").start()
            } catch (e: IOException) {
                e.printStackTrace()
                break
            }
        }
    }

    private fun handleClient(clientSocket: Socket) {

        // Процедура авторизации не имеет смысла без шифрования, так как стороннее лицо
        // может перехватить пакет с паролем и воспользоваться им для подключения к
        // серверу. Процедура авторизации была временно отключена.

//        if (!authorizeUser(clientSocket))
//            return

        handleCommands(clientSocket)
        while (true) {
            try {
                val message = readMessage(clientSocket)
                when (parseCommand(message)) {
                    UserAction.EVAL -> handleEval(clientSocket, message.toLowerCase().removePrefix("eval "))
                    UserAction.FACT -> handleFact(clientSocket, message.toLowerCase().removePrefix("fact "))
                    UserAction.SQRT -> handleSqrt(clientSocket, message.toLowerCase().removePrefix("sqrt "))
                    UserAction.MESSAGE -> handleSay(
                            clientSocket,
                            getName(clientSocket),
                            message.toLowerCase().removePrefix("say "))
                    UserAction.UNKNOWN -> handleUnknown(clientSocket)
                    UserAction.HELP -> handleCommands(clientSocket)
                    UserAction.HISTORY -> handleHistory(clientSocket)
                    UserAction.FUNCTIONS -> handleOperators(clientSocket)
                }
            } catch (e: IOException) {
                if (!clientSocket.isClosed)
                    clientSocket.close()
                clientDisconnected(clientSocket)
            }
        }
    }

    private fun handleHistory(clientSocket: Socket) {
        val login = getName(clientSocket)
        if (!histories.containsKey(login)){
            return
        }
        val userHistory = histories.get(login)
        if (userHistory == null){
            return
        }
        if (userHistory.isEmpty())
            return

        val stringBuilder = StringBuilder()
        stringBuilder.append("История ваших запросов (${userHistory.size}):\n")
        for ((index, oldResponse) in userHistory.withIndex()){
            stringBuilder.append("$index, ")
            if (oldResponse.length > CHUNK_SIZE){
                stringBuilder.append(oldResponse.take(CHUNK_SIZE-3)).append("...")
            } else {
                stringBuilder.append(oldResponse)
            }
            stringBuilder.append("\n")
        }
        sendMessage(clientSocket, stringBuilder.toString())
    }

    private fun getName(clientSocket: Socket): String {
        return activeUsers.get(clientSocket)!!
    }

    private fun authorizeUser(clientSocket: Socket): Boolean {
        var login: String
        try {
            while (true) {
                sendMessage(clientSocket, "Введите имя пользователя [a-zA-Z0-9] от 5 до 15 символов:")
                login = readMessage(clientSocket)
                if (!login.matches(Regex("[a-zA-Z0-9]{5,15}"))) {
                    sendMessage(clientSocket, "Неправильный формат имени")
                } else {
                    break
                }
            }
            if (userBase.hasUser(login)) {
                sendMessage(clientSocket, "Введите пароль")
                val pass = readMessage(clientSocket)
                if (userBase.checkPassword(login, pass)) {
                    activeUsers.put(clientSocket, login)
                    sendMessage(clientSocket, "Приветствуем вас, $login")
                    return true
                } else {
                    sendMessage(clientSocket, "Неверный пароль!\n" +
                            "Отключение соединения")
                    clientDisconnected(clientSocket)
                    return false
                }
            } else {
                sendMessage(clientSocket, "Данное имя ещё не зарегистрировано")
                while (true) {
                    sendMessage(clientSocket, "Придумайте пароль [a-zA-Z0-9] от 5 до 15 символов:")
                    val firstPass = readMessage(clientSocket)
                    if (!firstPass.matches(Regex("[a-zA-Z0-9]{5,15}"))) {
                        sendMessage(clientSocket, "Неверный формат пароля")
                        continue
                    }
                    sendMessage(clientSocket, "Введите повторно:")
                    val secondPass = readMessage(clientSocket)
                    if (firstPass == secondPass) {
                        userBase.addUser(login, firstPass)
                        activeUsers.put(clientSocket, login)
                        sendMessage(clientSocket, "Пользователь успешно зарегистрирован!\n" +
                                "Добро пожаловать. $login!")
                        return true
                    } else {
                        sendMessage(clientSocket, "Пароли не совпадают. Попробуйте ещё раз")
                    }
                }
            }
        } catch (e: IOException) {
            clientDisconnected(clientSocket)
            return false
        }
    }

    private fun clientDisconnected(clientSocket: Socket) {
        if (activeUsers.containsKey(clientSocket))
            activeUsers.remove(clientSocket)
        if (!clientSocket.isClosed)
            clientSocket.close()
    }

    private fun parseCommand(message: String): UserAction {
        if (lookForCommand(message, "say"))
            return UserAction.MESSAGE
        if (lookForCommand(message, "eval"))
            return UserAction.EVAL
        if (lookForCommand(message, "fact"))
            return UserAction.FACT
        if (lookForCommand(message, "sqrt"))
            return UserAction.SQRT
        if (message.toLowerCase() == "help")
            return UserAction.HELP
        if (message.toLowerCase() == "functions")
            return UserAction.FUNCTIONS
        if (message.toLowerCase() == "history")
            return UserAction.HISTORY
        return UserAction.UNKNOWN
    }

    private fun lookForCommand(message: String, command: String): Boolean {
        try {
            return message.toLowerCase().substring(0, command.length + 1) == "$command "
        } catch (e: StringIndexOutOfBoundsException) {
            return false
        }
    }

    private enum class UserAction {
        EVAL, SQRT, FACT, MESSAGE, UNKNOWN, HELP, FUNCTIONS, HISTORY
    }

    private fun handleOperators(clientSocket: Socket) {
        val stringBuilder = StringBuilder()
        stringBuilder.append("Операторы, доступные в рамках комманды eval:\n")
        for (operator in Utilities.SUPPORTED_OPERATIONS) {
            stringBuilder.append("$operator\n")
        }
        sendMessage(clientSocket, stringBuilder.toString())
    }

    private fun handleCommands(clientSocket: Socket) {
        val stringBuilder = StringBuilder()
        stringBuilder.append("Список доступных команд:\n")
        for (command in Constants.COMMANDS) {
            stringBuilder.append("${command.mnemonic.padEnd(Constants.CONSOLE_WIDTH / 3)}| ${command.description}\n")
        }
        sendMessage(clientSocket, stringBuilder.toString())
    }

    private fun handleUnknown(clientSocket: Socket) {
        sendMessage(clientSocket, "Неизвестная команда")
    }

    private fun handleSay(clientSocket: Socket, login: String, message: String) {
        Utilities.colorPrint("Сообщение от $login : $message\n", TextColors.ANSI_PURPLE)
        sendMessage(clientSocket, "Ваше сообщение доставлено.")
    }

    private fun handleSqrt(clientSocket: Socket, sqrt: String) {
        Thread(Runnable {
            val response = Utilities.calculateSqrt(sqrt)
            sendMessage(clientSocket, response)
            addHistory(getName(clientSocket), "SQRT($sqrt): $response")
        }, "Factorial-worker").start()
    }

    private fun addHistory(name: String, response: String) {
        if (!histories.containsKey(name)) {
            histories.put(name, mutableListOf())
            histories.get(name)!!.add(response)
            return
        }
        if (histories.get(name) == null) {
            histories.put(name, mutableListOf())
            histories.get(name)!!.add(response)
            return
        }
        histories.get(name)!!.add(response)
        return
    }

    private fun handleFact(clientSocket: Socket, factorial: String) {
        Thread(Runnable {
            val response = Utilities.calculateFactorial(factorial)
            addHistory(getName(clientSocket), "FACT($factorial): $response")
            sendMessage(clientSocket, "Факториал числа $factorial:")
            if (response.length < Constants.CHUNK_SIZE) {
                sendMessage(clientSocket, response)
            } else {
                var chunked = response
                while (chunked.isNotEmpty()) {
                    val chunk = chunked.take(Constants.CHUNK_SIZE)
                    sendMessage(clientSocket, chunk)
                    chunked = chunked.removePrefix(chunk)
                }
            }
        }, "Factorial-worker").start()
    }

    private fun handleEval(clientSocket: Socket, expression: String) {
        val response = evalExpression(expression)
        addHistory(getName(clientSocket), "EVAL: $expression = $response")
        sendMessage(clientSocket, response)
    }

}