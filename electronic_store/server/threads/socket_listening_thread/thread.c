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
    int client_sockfd = *(int*)arg;
    uint32_t packet_length;
    uint16_t packet_type;
    int number_read;
    void* packet;

    while (1) {
        //чтение размера пакета
        number_read = readn_(client_sockfd, &packet_length, SIZE_OF_PACKET_LENGTH);

        if(number_read < SIZE_OF_PACKET_LENGTH) {
            printf("ERROR reading packet length. Socket number: %d.\n", client_sockfd);
            list_of_clients_remove(client_sockfd);
            pthread_exit(0);
        }

        // принимаем пакет
        packet = malloc(packet_length);
        number_read = readn_(client_sockfd, packet, packet_length - SIZE_OF_PACKET_LENGTH);

        if( (uint32_t ) number_read < packet_length-SIZE_OF_PACKET_LENGTH) {
            printf("ERROR reading packet type. Socket number: %d.\n", client_sockfd);
            list_of_clients_remove(client_sockfd);
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


// создание потока
pthread_t create_listening_thread(int sockfd) {
    pthread_t listening_thread;

    if( pthread_create(&listening_thread, NULL, socket_listening_thread, (void *) &sockfd)) {
        return -1;
    }

    return listening_thread;
}