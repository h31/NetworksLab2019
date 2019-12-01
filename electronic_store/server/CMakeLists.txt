cmake_minimum_required(VERSION 2.8)
project(electronic_store)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread")

set(SOURCE_FILES main.c
        packet_handler/packet_handler.c packet_handler/packet_handler.h packet_handler/acknowledgment_types.h
        threads/console_listening_thread/thread.c threads/console_listening_thread/thread.h
        threads/socket_listening_thread/thread.c threads/socket_listening_thread/thread.h
        list_of_products/list_of_products.c list_of_products/list_of_products.h
        threads/accepting_thread/thread.c threads/accepting_thread/thread.h
        list_of_clients/list_of_clients.c list_of_clients/list_of_clients.h
        packet_types.h
        constants.h)
add_executable(electronic_store ${SOURCE_FILES})