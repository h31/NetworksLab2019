cmake_minimum_required(VERSION 2.8)
project(new_system_client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread")


find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(SOURCE_FILES main.c
        window_manager/window_manager.c window_manager/window_manager.h window_manager/user_choice.h
        packet_manager/packet_manager.c packet_manager/packet_manager.h packet_manager/packet_types.h
        constants.h packet_manager/acknowledgment_types.h packet_manager/error_types.h
        user_choice_handler/functions.c user_choice_handler/functions.h)
add_executable(new_system_client ${SOURCE_FILES})

target_link_libraries(new_system_client ${CURSES_LIBRARIES})