#include <sys/types.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "../../list_of_products/list_of_products.h"
#include "../../list_of_clients/list_of_clients.h"
#include "../../packet_handler/packet_handler.h"
#include "../../packet_types.h"
#include "../../constants.h"
#include "./thread.h"


static int readn_(int sockfd, void* dst, size_t len) {
    int total_number_read = 0;
    int local_number_read;

    while (len > 0) {
        local_number_read = read(sockfd, dst + total_number_read, len);

        if(local_number_read == 0) {
            return total_number_read;
        }

        if (local_number_read < 0) {
            return -1;
        }

        total_number_read += local_number_read;
        len -= local_number_read;
    }

    return total_number_read;
}


void* socket_listening_thread(void* arg) {
    int client_sockfd = ((Listening_thread_input*)arg)->sockfd;
    uint32_t packet_length;
    uint16_t packet_type;
    int number_read;
    void* packet;

    while (1) {
        //чтение размера пакета
        number_read = readn_(client_sockfd, &packet_length, SIZE_OF_PACKET_LENGTH);

        if(number_read < SIZE_OF_PACKET_LENGTH) {
            list_of_clients_remove(client_sockfd);
            free(arg);
            pthread_exit(0);
        }

        // принимаем пакет
        packet = malloc(packet_length);
        number_read = readn_(client_sockfd, packet, packet_length - SIZE_OF_PACKET_LENGTH);

        if( (uint32_t ) number_read < packet_length-SIZE_OF_PACKET_LENGTH) {
            list_of_clients_remove(client_sockfd);
            free(packet);
            free(arg);
            pthread_exit(0);
        }

        //анализируем код пакета
        if ( (packet_type = *(uint16_t*) packet) == ADD_PRODUCT_PACKET) {
            handle_add_product_packet(client_sockfd, packet);

        } else if (packet_type == BUY_PRODUCT_PACKET) {
            handle_buy_product_packet(client_sockfd, packet);

        } else if (packet_type == GET_LIST_OF_PRODUCTS_PACKET) {
            list_of_products_send(client_sockfd);
        }

        free(packet);
    }
}


Listening_thread_input* init_listening_thread_input_structure(int sockfd) {
    Listening_thread_input* new_input_structure = (Listening_thread_input*) malloc(sizeof(Listening_thread_input));

    new_input_structure->sockfd = sockfd;

    return new_input_structure;
}


// создание потока
pthread_t create_listening_thread(int sockfd) {
    pthread_t listening_thread;

    Listening_thread_input* listening_thread_input = init_listening_thread_input_structure(sockfd);

    if( pthread_create(&listening_thread, NULL, socket_listening_thread, listening_thread_input)) {
        return -1;
    }

    return listening_thread;
}