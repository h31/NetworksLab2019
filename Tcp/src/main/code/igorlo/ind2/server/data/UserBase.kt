package igorlo.ind2.server.data

import java.lang.IllegalStateException
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.ConcurrentMap

class UserBase {
    private val table: ConcurrentMap<String, User> = ConcurrentHashMap()

    fun hasUser(login: String): Boolean {
        return table.containsKey(login)
    }

    fun addUser(login: String, password: String){
        if (hasUser(login))
            throw IllegalStateException("Пользователь уже существует")
        table[login] = User(login, password)
    }

    fun checkPassword(login: String, password: String): Boolean {
        if (!table.containsKey(login))
            return false
        if (table[login]!!.pass == password) {
            return true
        }
        return false
    }
}