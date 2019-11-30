package koin

import data.ConfigProvider
import data.impl.ConfigProviderImpl
import org.koin.dsl.module

val myModule = module {
    single<ConfigProvider> { ConfigProviderImpl(getProperty("zones_path")) }
}
