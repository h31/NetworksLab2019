package args4j

import domain.model.DNSName
import domain.model.enums.DNSQueryType
import org.kohsuke.args4j.CmdLineParser
import org.kohsuke.args4j.OptionHandlerRegistry

object ArgumentsParser {

    init {
        OptionHandlerRegistry.getRegistry().registerHandler(DNSName::class.java, DNSNameArgumentHandler::class.java)
        OptionHandlerRegistry.getRegistry().registerHandler(DNSQueryType::class.java, DNSQueryTypeHandler::class.java)
    }

    fun parse(args: Array<String>) = Args().also {
        CmdLineParser(it).parseArgument(*args)
    }

}
