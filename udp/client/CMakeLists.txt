cmake_minimum_required(VERSION 2.8)
project(udp_client_linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread")

set(SOURCE_FILES main.c main.c
        console_handler/console_handler.h console_handler/functions.c constants.h console_handler/user_choice.h
        packet_creator/functions.c packet_creator/packet_creator.h packet_creator/packet_types.h
        upload_file_handler/functions.c upload_file_handler/upload_handler.h
        packet_handler/functions.c packet_handler/packet_handler.h
        download_file_handler/download_handler.h
        download_file_handler/functions.c)

add_executable(client_linux ${SOURCE_FILES})