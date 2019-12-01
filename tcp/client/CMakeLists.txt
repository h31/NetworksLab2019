cmake_minimum_required(VERSION 2.8)
project(tcp_client)

find_library(PTHREAD_LIBRARY pthread)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

set(SOURCE_FILES main.c protocol.c)
add_executable(client ${SOURCE_FILES})

target_link_libraries(client ${PTHREAD_LIBRARY})
