cmake_minimum_required(VERSION 2.8)
project(udp_server_linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

set(SOURCE_FILES main.c
        packet_types.h
        list_of_clients/user_modes.h
        constants.h
        list_of_clients/functions.c list_of_clients/list_of_clients.h
        packet_creator/functions.c packet_creator/packet_creator.h
        packet_handler/functions.c packet_handler/packet_handler.h)

add_executable(server_linux ${SOURCE_FILES})