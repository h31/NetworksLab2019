package igorlo.ind2.server

import igorlo.ind2.Constants.PORT
import igorlo.ind2.server.Handlers.handleCommands
import igorlo.ind2.server.Handlers.handleFact
import igorlo.ind2.server.Handlers.handleHistory
import igorlo.ind2.server.Handlers.handleOperators
import igorlo.ind2.server.Handlers.handleSay
import igorlo.ind2.server.Handlers.handleSqrt
import igorlo.ind2.server.Handlers.handleUnknown
import igorlo.ind2.server.data.UserBase
import igorlo.util.Exchange.readMessage
import igorlo.util.Exchange.sendMessage
import igorlo.util.Utilities.evalExpression
import java.io.IOException
import java.net.ServerSocket
import java.net.Socket

class Server {

    private val acceptingSocket: ServerSocket = ServerSocket(PORT)
    private val userBase = UserBase()

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
        if (!authorizeUser(clientSocket))
            return
        handleCommands(clientSocket)
        while (true) {
            try {
                val message = readMessage(clientSocket)
                when (parseCommand(message)) {
                    UserAction.EVAL -> sendMessage(
                            clientSocket,
                            evalExpression(message.toLowerCase().removePrefix("eval "))
                    )
                    UserAction.FACT -> handleFact(clientSocket, message.toLowerCase().removePrefix("fact "))
                    UserAction.SQRT -> handleSqrt(clientSocket, message.toLowerCase().removePrefix("sqrt "))
                    UserAction.MESSAGE -> handleSay(clientSocket, message.toLowerCase().removePrefix("say "))
                    UserAction.UNKNOWN -> handleUnknown(clientSocket)
                    UserAction.HELP -> handleCommands(clientSocket)
                    UserAction.HISTORY -> handleHistory(clientSocket)
                    UserAction.FUNCTIONS -> handleOperators(clientSocket)
                }
            } catch (e: IOException) {
                if (!clientSocket.isClosed)
                    clientSocket.close()
//                clientDisconnected(clientSocket)
            }
        }
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
                        sendMessage(clientSocket, "Пользователь успешно зарегистрирован!\n" +
                                "Добро пожаловать!")
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

}