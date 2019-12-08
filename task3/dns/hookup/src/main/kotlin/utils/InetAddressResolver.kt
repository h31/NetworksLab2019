package utils

import java.net.InetAddress

object InetAddressResolver {

    @Throws(exceptionClasses = [IllegalArgumentException::class])
    fun resolve(addressString: String): InetAddress {
        try {
            return InetAddress.getByName(addressString)
        } catch (e: Throwable) {
            throw IllegalArgumentException("Can't resolve inet address : '$addressString'. Format is address:port")
        }
    }

}
