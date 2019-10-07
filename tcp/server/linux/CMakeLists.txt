cmake_minimum_required(VERSION 2.8)
project(server_linux)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pthread")

set(SOURCE_FILES main.c threads/listening_thread/thread.c threads/listening_thread/thread.h pipe/methods.c pipe/pipe.h threads/writing_thread/thread.c threads/writing_thread/thread.h user_info_list/methods.c user_info_list/user_info_list.h message_buffer/message_buffer.c message_buffer/message_buffer.h)
add_executable(server_linux ${SOURCE_FILES})
