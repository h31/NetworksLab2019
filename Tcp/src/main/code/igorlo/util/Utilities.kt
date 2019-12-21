package igorlo.util

import igorlo.TextColors

object Utilities {

    fun colorPrint(text: String, color: String) {
        print("${color}$text${TextColors.ANSI_RESET}")
    }

    data class Command(val mnemonic: String, val description: String)

}