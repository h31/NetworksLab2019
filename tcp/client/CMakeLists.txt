cmake_minimum_required(VERSION 2.8)
project(client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread")
set(SRC_LIST client.c)

add_executable(${PROJECT_NAME} ${SRC_LIST})
