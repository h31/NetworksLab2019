package igorlo.dns.basic

import igorlo.dns.basic.DnsClient

object Main {

    @JvmStatic
    fun main(args: Array<String>) {
        DnsClient().run()
    }

}