package data.impl

import data.ConfigProvider
import domain.model.DNSName
import domain.model.DNSRRData
import domain.model.DNSResourceRecord
import domain.model.enums.DNSKlass
import domain.model.enums.DNSQueryType
import org.koin.core.logger.Logger
import utils.IP
import utils.inject
import java.io.BufferedInputStream

class ConfigProviderImpl(
        private val path: String
) : ConfigProvider {

    companion object {
        private const val MAX_TOKENS_SIZE = 6
        private const val MIN_TOKENS_SIZE = 5
        private const val NAME_INDEX = 0
        private const val TTL_INDEX = 1
        private const val KLASS_INDEX = 2
        private const val TYPE_INDEX = 3
    }

    private val logger: Logger by inject()

    private val zoneInfoFile = try {
        logger.info("Trying to read zone info from: $path")
        BufferedInputStream(ClassLoader.getSystemResourceAsStream(path)).bufferedReader()
    } catch (t: Throwable) {
        logger.error("Error on loading zones info from $path. Cause: ${t.localizedMessage}")
        null
    }

    private val delimiter = Regex("[\t| ]+")

    override fun provideZoneInfo(): List<DNSResourceRecord> =
            getBufferedReaderLines().mapNotNull {
                try {
                    parseResourceRecord(it)
                } catch (e: Throwable) {
                    logger.error("Error on parsing line: $it")
                    null
                }
            }.also {
                logger.info("Parsed ${it.size} records from $path")
            }

    private fun getBufferedReaderLines(): List<String> = try {
        zoneInfoFile?.readLines() ?: emptyList()
    } catch (t: Throwable) {
        logger.error("Error on getting lines from $path")
        emptyList()
    }

    @Throws(exceptionClasses = [IllegalStateException::class, NumberFormatException::class, IllegalArgumentException::class])
    private fun parseResourceRecord(record: String): DNSResourceRecord {
        val tokens = record.split(delimiter)
        check(tokens.size in MIN_TOKENS_SIZE..MAX_TOKENS_SIZE) { "Illegal resource record: $record" }
        val name = tokens[NAME_INDEX]
        val ttl = tokens[TTL_INDEX].toInt()
        val klass: DNSKlass = DNSKlass.valueOf(tokens[KLASS_INDEX])
        val type = DNSQueryType.valueOf(tokens[TYPE_INDEX])
        val data = parseDNSRRData(tokens.drop(MIN_TOKENS_SIZE - 1), type)
        return DNSResourceRecord(
                name = createDNSName(name),
                type = DNSQueryType.of(type.value),
                klass = klass,
                ttl = ttl,
                dataLength = data.getDataLength(),
                data = data
        )
    }

    @Throws(exceptionClasses = [NumberFormatException::class, IllegalArgumentException::class])
    private fun parseDNSRRData(data: List<String>, type: DNSQueryType): DNSRRData = when (type) {
        DNSQueryType.A -> DNSRRData.A(IP.stringIPv4ToInt(data.first()))
        DNSQueryType.NS -> DNSRRData.NS(createDNSName(data.first()))
        DNSQueryType.CNAME -> DNSRRData.CName(createDNSName(data.first()))
        DNSQueryType.H_INFO -> DNSRRData.HInfo(createDNSName(data.first()))
        DNSQueryType.MX -> DNSRRData.MX(data.first().toShort(), createDNSName(data.last()))
        else -> DNSRRData.Undefined
    }

    @Throws(exceptionClasses = [IllegalStateException::class])
    private fun createDNSName(name: String): DNSName {
        val nameBuilder = DNSName.Builder()
        name.split(".").forEach(nameBuilder::addMark)
        return nameBuilder.build()
    }

}
