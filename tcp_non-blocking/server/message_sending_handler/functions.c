#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "../constants.h"
#include "../list_of_users/list_of_users.h"


void add_date_to_full_message(char* dst) {
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    char time[TIME_SIZE + 1];
    strftime (time, TIME_SIZE + 1, "<%H:%M:%S>", timeinfo);

    strncat(dst, time, TIME_SIZE);
}


void add_nickname_to_full_message(char* dst, int user_index) {
    strncat(dst, list_of_users_get_name(user_index), MAX_NICKNAME_SIZE);
    char* r = ": ";
    strncat(dst, r, strlen(r));
}


void add_message_to_full_message(char* dst, int user_index) {
    char* message = list_of_users_get_message(user_index);
    strncat(dst, message, strnlen(message, MAX_RECEIVED_MESSAGE_SIZE));
    list_of_users_clean_message(user_index);
}


void send_message_to_client(int client_index, char* message, int message_size) {
    int socket_number = list_of_users_get_socket_number(client_index);

    if (socket_number > 0) {
        if (write(socket_number, &message_size, sizeof(message_size)) < 0) {
            fprintf(stderr, "ERROR writing to socket\n");
        }
        // sending message
        if (write(socket_number, message, strlen(message)) < 0) {
            fprintf(stderr, "ERROR writing to socket\n");
        }
    }
}

void send_message_to_clients(int user_index) {
    //определить отправляемое сообщение
    char full_message[TIME_SIZE + MAX_NICKNAME_SIZE + MAX_RECEIVED_MESSAGE_SIZE];
    bzero(full_message, TIME_SIZE + MAX_NICKNAME_SIZE + MAX_RECEIVED_MESSAGE_SIZE);

    // добавить дату
    add_date_to_full_message(full_message);

    // добавить имя
    add_nickname_to_full_message(full_message, user_index);

    // добавить сообщение
    add_message_to_full_message(full_message, user_index);

    //отправить клиентам
    int message_size = (int) strlen(full_message);
    for (int clientIndex = 0; clientIndex < MAX_NUMBER_OF_CONNECTED_USERS; ++clientIndex) {
        send_message_to_client(clientIndex, full_message, message_size);
    }

}


void message_sending_handler_send_messages(void) {
    for (int i = 0; i < MAX_NUMBER_OF_CONNECTED_USERS; ++i) {
        if ((int) strlen(list_of_users_get_message(i)) == list_of_users_get_message_length(i)) {
            send_message_to_clients(i);
        }
    }
}