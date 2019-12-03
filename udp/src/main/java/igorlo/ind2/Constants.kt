package igorlo.ind2

import org.apache.log4j.Level

object Constants {
    val DEFAULT_LOGGING_LEVEL = Level.WARN
    const val PORT = 6789
    const val CHUNK_SIZE = 150
    const val CONSOLE_WIDTH = 120
    val COMMANDS = listOf(
            Command("help", "Выводит список доступных команд"),
            Command("functions", "Доступных математических функций"),
            Command("eval <expression>", "Считает значение выражения"),
            Command("fact <number>", "Считает указанный факториал целого числа"),
            Command("sqrt <number>", "Считает квадратный корень из указанного числа"),
            Command("say <message>", "Отправить сообщение серверу"),
            Command("history", "Получить историю результатов запросов")
    )
}