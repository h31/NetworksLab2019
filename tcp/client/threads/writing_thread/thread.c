#include "thread.h"
#include <ncurses.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "../../main_window/main_window.h"
#include "../../constants.h"

// add char to string
void append_char(char new_char, char* dst, int max_dst_length) {
    int i = 0;

    while (dst[i] != '\000') {
        i++;
    }

    if (i != max_dst_length - 1) {
        dst[i] = new_char;
    }
}


// delete last character from string
void delete_last_char(char* string) {
    int i = 0;

    while (string[i] != '\0') {
        i++;
    }

    if (i != 0) {
        string[i-1] = '\0';
    }
}


void send_message_to_server(int sockfd, void* message, size_t size) {
    //sending message size
    int message_size = (int) strlen(message);
    if (write(sockfd, &message_size, sizeof(message_size)) < 0) {
        fprintf(stderr, "ERROR writing in socket\n");
    }
    // sending message
    if (write(sockfd, message, size) < 0) {
        fprintf(stderr, "ERROR writing in socket\n");
    }
}


// ask user for nickname and send it to server
void send_nickname_to_server(int sockfd) {
    int input_char = 0;
    char* asking_message = "Enter nickname: ";
    char request_and_nickname[strlen(asking_message) + MAX_NICKNAME_SIZE + 1];
    strncat(request_and_nickname, asking_message, strlen(asking_message));
    char nickname[MAX_NICKNAME_SIZE + 1];

    // reading nickname
    while (input_char != '\n') {
        char *tmp = strdup(request_and_nickname);

        main_window_write_to_input_window(strncat(tmp, nickname, MAX_NICKNAME_SIZE + 1));
        input_char = main_window_get_char();

        if (input_char == KEY_BACKSPACE) {
            delete_last_char(nickname);
        } else if(input_char != '\n'){
            append_char(input_char, nickname, MAX_NICKNAME_SIZE + 1);
        }

        free(tmp);
    }


    send_message_to_server(sockfd, nickname, strlen(nickname));
    main_window_clear_input_window();
}

// чтение сообщений пользователя и отправка их на сервер
void* writing_thread(void* arg) {
    int input_char;
    char user_message[((Writing_thread_input*) arg) -> input_line_size + 1];
    bzero(user_message, ((Writing_thread_input*) arg) -> input_line_size + 1);


    send_nickname_to_server(((Writing_thread_input *) arg)->sockfd);

    // reading user message
    while (1) {
        input_char = main_window_get_char();

        switch (input_char) {
            case KEY_BACKSPACE: {
                delete_last_char(user_message);
                break;
            }
            case '\n': {
                send_message_to_server(((Writing_thread_input *) arg)->sockfd, user_message, strlen(user_message));
                bzero(user_message, ((Writing_thread_input*) arg) -> input_line_size + 1);
                break;
            }
            default: {
                append_char(input_char, user_message, ((Writing_thread_input*) arg) -> input_line_size + 1);
                break;
            }
        }

        main_window_write_to_input_window(user_message);
    }

}


Writing_thread_input* writing_thread_init_input_structure(int sockfd, int input_line_size) {
    Writing_thread_input* new_input_structure = (Writing_thread_input*) malloc(sizeof(Writing_thread_input));

    new_input_structure -> sockfd = sockfd;
    new_input_structure -> input_line_size = input_line_size;

    return new_input_structure;
}


pthread_t create_writing_thread(int sockfd, int input_line_size) {
    pthread_t _writing_thread;
    Writing_thread_input* writing_thread_input = writing_thread_init_input_structure(sockfd, input_line_size);

    if( pthread_create(&_writing_thread, NULL, writing_thread, (void *) writing_thread_input)) {
        fprintf(stderr, "ERROR creating writing thread\n");
    }

    return _writing_thread;
}
