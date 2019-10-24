cmake_minimum_required(VERSION 2.8)
project(client)

set(CMAKE_C_FLAGS "-g -pthread ${CMAKE_C_FLAGS} -Wall -Wextra")

set(SOURCE_FILES main.c ../common-utils/common.c ../common-utils/errors.c ../common-utils/inet_utils.c )
add_executable(client ${SOURCE_FILES})