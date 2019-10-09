cmake_minimum_required(VERSION 2.8)
project(client_linux)

find_library(PTHREAD_LIBRARY pthread)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

set(SOURCE_FILES main.c)
add_executable(client_linux ${SOURCE_FILES})

target_link_libraries(client_linux ${PTHREAD_LIBRARY})
