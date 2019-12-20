package igorlo.ind2.server.data

import org.jetbrains.annotations.NotNull

class User(
        @NotNull val login: String,
        @NotNull val pass: String
) : Comparable<User> {

    override fun compareTo(other: User): Int {
        return login.compareTo(other.login)
    }

}