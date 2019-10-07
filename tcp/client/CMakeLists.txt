cmake_minimum_required(VERSION 2.8)
project(client_linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread -lncurses")


find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(SOURCE_FILES main.c threads/listening_thread/thread.c threads/listening_thread/thread.h threads/writing_thread/thread.c threads/writing_thread/thread.h main_window/main_window.c main_window/main_window.h constants.h)
add_executable(client_linux ${SOURCE_FILES})

target_link_libraries(client_linux ${CURSES_LIBRARIES})