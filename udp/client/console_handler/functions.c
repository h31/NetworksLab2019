#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../constants.h"
#include "user_choice.h"


void console_print_possible_user_actions() {
    fprintf(stdout, "Choose:\n");
    fprintf(stdout, "%d) Download file.\n", DOWNLOAD_FILE_CHOICE);
    fprintf(stdout, "%d) Upload file.\n", UPLOAD_FILE_CHOICE);
    fprintf(stdout, "%d) Quit\n", QUIT_CHOICE);
}


int console_get_user_choice() {
    char buffer[sizeof(int)];

    console_print_possible_user_actions();
    fgets(buffer, sizeof(int), stdin);

    return atoi(buffer);
}


void remove_new_line_char_(char* string, int string_size) {
    for (int i = 0; i < string_size; ++i) {
        if (string[i] == '\n') {
            string[i] = '\0';
            return;
        }
    }
}


void console_get_file_path(char* dst, int number_to_read, char* request_message) {
    bzero(dst, number_to_read);

    fprintf(stdout, "%s: ", request_message);
    fgets(dst, number_to_read, stdin);
    remove_new_line_char_(dst, MAX_FILE_PATH_SIZE);
}


void console_print_success_download_message(char* downloaded_file) {
    fprintf(stdout, "File '%s' was successfully downloaded.\n", downloaded_file);
}


void console_print_success_upload_message(char* uploaded_file) {
    fprintf(stdout, "File '%s' was successfully uploaded.\n", uploaded_file);
}


void console_print_error_message(char* error_message) {
    fprintf(stderr, "ERROR: %s.\n", error_message);
}


void console_print_error(int error_code, char* error_message) {
    fprintf(stdout, "ERROR: error code: %d; error message: %s\n", error_code, error_message);
}