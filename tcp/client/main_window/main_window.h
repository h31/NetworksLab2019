
#ifndef CLIENT_LINUX_MAIN_WINDOW_H
#define CLIENT_LINUX_MAIN_WINDOW_H

void main_window_init(void);

void main_window_init_mutex(void);

void main_window_write_to_output_window(char* messages[], int messages_size);

void main_window_write_to_input_window(char* string);

void main_window_clear_input_window(void);

int main_window_get_char(void);

void main_window_exit(void);

#endif //CLIENT_LINUX_MAIN_WINDOW_H
