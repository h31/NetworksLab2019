cmake_minimum_required(VERSION 2.8)
project(udp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

set(USED_FILES file.c file.h
				fsm.c fsm.h
				netudp.c netudp.h
				tftp.c tftp.h)
set(SOURCE_FILE_SV server/server.c ${USED_FILES})
				
set(SOURCE_FILE_CL client/client.c ${USED_FILES})

add_executable(server_out ${SOURCE_FILE_SV})
add_executable(client_out ${SOURCE_FILE_CL})
