
#ifndef UDP_CLIENT_LINUX_CONSOLE_HANDLER_H
#define UDP_CLIENT_LINUX_CONSOLE_HANDLER_H

int console_get_file_path(char* dst,int number_to_read, char* request_message);

void console_print_success_download_message(char* downloaded_file);

void console_print_success_upload_message(char* uploaded_file);

void console_print_error(int error_code, char* error_message);

void console_print_error_message(char* error_message);

void console_print_possible_user_actions(void);

int console_get_user_choice(void);

#endif //UDP_CLIENT_LINUX_CONSOLE_HANDLER_H
