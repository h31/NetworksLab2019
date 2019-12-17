package igorlo.util

import igorlo.TextColors
import java.lang.NumberFormatException
import java.math.BigDecimal
import java.util.*
import javax.script.ScriptEngineManager
import javax.script.ScriptException

object Utilities {

    private val factorialResults: MutableMap<Long, String> = HashMap()

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
            return "0"
        }
    }

    fun calculateFactorial(factorial: String): String {
        try {
            val bigFactorial = factorial.toLong()
            if (factorialResults.containsKey(bigFactorial)){
                if (factorialResults[bigFactorial] != null){
                    return factorialResults[bigFactorial]!!
                }
            }
            var bigNumber = BigDecimal.ONE
            if (bigFactorial == 0L) {
                return "0"
            }
            for (i in 1..bigFactorial) {
                bigNumber = bigNumber.multiply(BigDecimal(i))
            }
            factorialResults.put(bigFactorial, bigNumber.toString())
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

//    fun ipAddressToString(ipAddress: ByteArray): String {
//        val builder = StringBuilder()
//                .append("IP[")
//                .append(MessageUtils.intFromSignedByte(ipAddress[0]))
//                .append(".")
//                .append(MessageUtils.intFromSignedByte(ipAddress[1]))
//                .append(".")
//                .append(MessageUtils.intFromSignedByte(ipAddress[2]))
//                .append(".")
//                .append(MessageUtils.intFromSignedByte(ipAddress[3]))
//                .append("]")
//        return builder.toString()
//    }

    data class Command(val mnemonic: String, val description: String)
}