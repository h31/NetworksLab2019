cmake_minimum_required(VERSION 2.8)
project(tcp_server)

find_library(PTHREAD_LIBRARY pthread)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

set(SOURCE_FILES main.c protocol.c)
add_executable(server ${SOURCE_FILES})

target_link_libraries(server ${PTHREAD_LIBRARY})
