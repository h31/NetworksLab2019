package koin

import data.ConfigProvider
import data.impl.ConfigProviderImpl
import org.koin.core.logger.Logger
import org.koin.core.logger.PrintLogger
import org.koin.dsl.module
import services.Server
import services.impl.ServerImpl

val appModule = module {
    single<ConfigProvider> { ConfigProviderImpl(getProperty("zones_path", "localhost.zone")) }
    single<Server> { ServerImpl() }
    single<Logger> { PrintLogger() }
}
