cmake_minimum_required(VERSION 2.8)
project(client_linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread")

set(SOURCE_FILES main.c)
set(HEADER_FILES ../common/common.h)
add_executable(client_linux ${HEADER_FILES} ${SOURCE_FILES})