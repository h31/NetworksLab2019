package args4j

import domain.model.enums.DNSQueryType
import org.kohsuke.args4j.CmdLineParser
import org.kohsuke.args4j.OptionDef
import org.kohsuke.args4j.spi.OneArgumentOptionHandler
import org.kohsuke.args4j.spi.Setter

class DNSQueryTypeHandler(
        parser: CmdLineParser?,
        option: OptionDef?,
        setter: Setter<in DNSQueryType>?
) : OneArgumentOptionHandler<DNSQueryType>(parser, option, setter) {

    override fun parse(argument: String?): DNSQueryType {
        checkNotNull(argument) { "Argument can't be null" }
        try {
            return DNSQueryType.valueOf(argument)
        } catch (t: IllegalArgumentException) {
            throw IllegalArgumentException(
                    "Illegal type for '-t' option. Use one of the following types: ${DNSQueryType.values().toList()}"
            )
        }
    }

}
