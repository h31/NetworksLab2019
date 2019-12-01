#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "../list_of_products/list_of_products.h"
#include "./acknowledgment_types.h"
#include "../packet_types.h"
#include "../constants.h"


//отправка пакета с подтверждением
void send_acknowledgment_packet(int client_sockfd, uint16_t ack_type, uint32_t count) {
    uint32_t packet_length = SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_ACK_TYPE + SIZE_OF_PACKET_COUNT;
    uint16_t packet_type = ACKNOWLEDGMENT_PACKET;
    void* packet = malloc(packet_length);

    memcpy(packet, &packet_length, SIZE_OF_PACKET_LENGTH);
    memcpy(packet + SIZE_OF_PACKET_LENGTH, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE, &ack_type, SIZE_OF_PACKET_ACK_TYPE);
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_ACK_TYPE, &count, SIZE_OF_PACKET_COUNT);

    if (write(client_sockfd, packet, packet_length) < packet_length) {
        printf("ERROR sending acknowledgment packet to client. Client socket number: %d.\n", client_sockfd);
    }

    free(packet);
}


void send_error_packet(int client_sockfd, char* msg, ulong msg_size) {
    uint32_t packet_length = SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + msg_size;
    uint16_t packet_type = ERROR_PACKET;
    void* packet = malloc(packet_length);

    memcpy(packet, &packet_length, SIZE_OF_PACKET_LENGTH);
    memcpy(packet + SIZE_OF_PACKET_LENGTH, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet+SIZE_OF_PACKET_LENGTH+SIZE_OF_PACKET_TYPE, msg, msg_size);

    if (write(client_sockfd, packet, packet_length) < packet_length ) {
        printf("ERROR sending error packet to client. Client socket number: %d.\n", client_sockfd);
    }

    free(packet);
}


void handle_add_product_packet(int client_sockfd, void* packet) {
    char* name = (char*) packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_COUNT + SIZE_OF_PACKET_PRICE;
    uint32_t price = *(uint32_t*) (packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_COUNT);
    uint32_t count = *(uint32_t*) (packet + SIZE_OF_PACKET_TYPE);

    if (list_of_products_add(name, count, price) == -1 ) {
        char* error_msg = "duplicate product with another cost";
        send_error_packet(client_sockfd, error_msg, strlen(error_msg));
    } else {
        send_acknowledgment_packet(client_sockfd, PRODUCT_WAS_ADDED, count);
    }
}


void handle_buy_product_packet(int client_sockfd, void* packet) {
    char* name = (char*) packet + SIZE_OF_PACKET_TYPE + SIZE_OF_PACKET_COUNT;
    uint32_t count = *(uint32_t*) packet + SIZE_OF_PACKET_TYPE;
    int number_removed;

    if ( (number_removed = list_of_products_remove(name, count)) == -1) {
        char* err_msg = "no such product";
        send_error_packet(client_sockfd, err_msg, strlen(err_msg));
    } else {
        send_acknowledgment_packet(client_sockfd, PRODUCT_WAS_BOUGHT, number_removed);
    }
}