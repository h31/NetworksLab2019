cmake_minimum_required(VERSION 2.8)

project(client_server)

find_library(PTHREAD_LIBRARY pthread)

add_executable(client client/main.c)
add_executable(server server/main.c header.h)


target_link_libraries(server ${PTHREAD_LIBRARY})
target_link_libraries(client ${PTHREAD_LIBRARY})