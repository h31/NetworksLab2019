package utils

import org.koin.core.context.GlobalContext

inline fun <reified T> inject() = GlobalContext.get().koin.inject<T>()
