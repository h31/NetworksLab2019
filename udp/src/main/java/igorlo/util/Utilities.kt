package igorlo.util

import igorlo.TextColors
import java.lang.NumberFormatException
import java.math.BigDecimal
import javax.script.ScriptEngineManager
import javax.script.ScriptException

object Utilities {

    val SUPPORTED_OPERATIONS = listOf(
            "abs",
            "cos",
            "sin",
            "tan",
            "tanh",
            "acos",
            "asin",
            "acosh",
            "asinh",
            "atanh",
            "cbrt",
            "ceil",
            "clz32",
            "exp",
            "floor",
            "fround",
            "log",
            "log10",
            "log2",
            "max",
            "min",
            "random",
            "round",
            "sign",
            "sqrt",
            "trunc"
    )

    fun colorPrint(text: String, color: String) {
        print("${color}$text${TextColors.ANSI_RESET}")
    }

    fun calculateSqrt(sqrt: String): String {
        try {
            val value = sqrt.toDouble()
            return kotlin.math.sqrt(value).toString()
        } catch (e: NumberFormatException) {
            return "Некорректное число"
        }
    }

    fun calculateFactorial(factorial: String): String {
        try {
            var bigNumber = BigDecimal.ONE
            val bigFactorial = factorial.toLong()
            if (bigFactorial == 0L) {
                return "0"
            }
            for (i in 1..bigFactorial) {
                bigNumber = bigNumber.multiply(BigDecimal(i))
            }
            return bigNumber.toString()
        } catch (e: NumberFormatException) {
            return "ERROR" //TODO
        }
    }

    fun evalExpression(expression: String): String {
        try {
            val convertedString = verySmartReplacement(expression)
            val engine = ScriptEngineManager().getEngineByExtension("js")
            val result = engine.eval(convertedString)
            return result.toString()
        } catch (e: ScriptException) {
            return "Не удалось вычислить выражение."
        }
    }

    private fun verySmartReplacement(expression: String): String {
        var convertedExpression = expression
        for (operation in SUPPORTED_OPERATIONS) {
            convertedExpression = convertedExpression.replace(operation, "Math.$operation")
        }
        return convertedExpression
    }

    data class Command(val mnemonic: String, val description: String)
}