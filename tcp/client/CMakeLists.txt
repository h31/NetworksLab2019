cmake_minimum_required(VERSION 2.8)
project(client)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

find_package (Threads)
set(SOURCE_FILES client.c)
add_executable(client ${SOURCE_FILES})

target_link_libraries (client ${CMAKE_THREAD_LIBS_INIT})
