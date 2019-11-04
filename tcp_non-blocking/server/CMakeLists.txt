cmake_minimum_required(VERSION 2.8)
project(server_linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

set(SOURCE_FILES main.c list_of_users/functions.c list_of_users/list_of_users.h constants.h list_of_connections/functions.c list_of_connections/list_of_connections.h message_sending_handler/functions.c message_sending_handler/messages_sending_handler.h message_receiving_handler/functions.c message_receiving_handler/message_receiving_handler.h)
add_executable(server_linux ${SOURCE_FILES})