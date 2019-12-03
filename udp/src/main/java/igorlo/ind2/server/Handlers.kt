package igorlo.ind2.server

import igorlo.TextColors
import igorlo.ind2.Constants.CHUNK_SIZE
import igorlo.ind2.Constants.COMMANDS
import igorlo.ind2.Constants.CONSOLE_WIDTH
import igorlo.util.Utilities.SUPPORTED_OPERATIONS
import igorlo.util.Utilities.calculateFactorial
import igorlo.util.Utilities.calculateSqrt
import igorlo.util.Utilities.colorPrint
import java.net.Socket

import igorlo.util.Exchange.sendMessage as send

