#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "constants.h"
#include "./console_handler/user_choice.h"
#include "./console_handler/console_handler.h"
#include "./upload_file_handler/upload_handler.h"
#include "./packet_handler/packet_handler.h"
#include "./download_file_handler/download_handler.h"


void check_number_of_args(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
}


int main(int argc, char* argv[]) {
    int user_choice;
    char remote_file_path[MAX_FILE_PATH_SIZE];
    char local_file_path[MAX_FILE_PATH_SIZE];

    check_number_of_args(argc, argv);

    packet_handler_init(argv[1], (uint16_t) atoi(argv[2]));

    while ( (user_choice = console_get_user_choice()) != QUIT_CHOICE) {
        if (user_choice == DOWNLOAD_FILE_CHOICE) {
            console_get_file_path(remote_file_path, MAX_FILE_PATH_SIZE,"Enter remote file path");
            console_get_file_path(local_file_path, MAX_FILE_PATH_SIZE,"Enter local file path");
            download_file(remote_file_path, local_file_path);
        } else if (user_choice == UPLOAD_FILE_CHOICE) {
            console_get_file_path(local_file_path, MAX_FILE_PATH_SIZE,"Enter file path");
            upload_file(local_file_path);
        }
    }

} 
