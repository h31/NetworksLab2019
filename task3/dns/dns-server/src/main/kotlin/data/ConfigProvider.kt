package data

import domain.model.DNSResourceRecord

interface ConfigProvider {

    fun provideZoneInfo(): List<DNSResourceRecord>

}
