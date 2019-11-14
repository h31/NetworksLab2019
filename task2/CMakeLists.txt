cmake_minimum_required(VERSION 3.9)
project(NetworksLab2019)

set(CMAKE_CXX_STANDARD 11)

add_executable(
        NetworksLab2019
        common/common.c
        common/common.h
        server/server.c
        server/server.h
        server/clients_handler.c
        server/clients_handler.h
        server/printer.c server/printer.h server/fds_worker.c server/fds_worker.h server/time_utils.c server/time_utils.h)
