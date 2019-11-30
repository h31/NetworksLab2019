package args4j

import domain.model.DNSName
import domain.model.enums.DNSQueryType
import org.kohsuke.args4j.Argument
import org.kohsuke.args4j.Option

class Args {

    @Argument(required = true)
    lateinit var name: DNSName

    @Option(name = "-t", required = true, handler = DNSQueryTypeHandler::class)
    lateinit var type: DNSQueryType

    @Option(name = "-s", required = true)
    lateinit var serverAddress: String

}
