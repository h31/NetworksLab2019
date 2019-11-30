package args4j

import domain.model.DNSName
import org.kohsuke.args4j.CmdLineParser
import org.kohsuke.args4j.OptionDef
import org.kohsuke.args4j.spi.OneArgumentOptionHandler
import org.kohsuke.args4j.spi.Setter

class DNSNameArgumentHandler(
        parser: CmdLineParser?,
        option: OptionDef?,
        setter: Setter<in DNSName>?
) : OneArgumentOptionHandler<DNSName>(parser, option, setter) {

    override fun parse(argument: String?): DNSName {
        checkNotNull(argument) { "Dns name can't be null" }
        val builder = DNSName.Builder()
        argument.split(".").forEach(builder::addMark)
        return builder.build()
    }

}
