#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../packet_types.h"
#include "../constants.h"


int create_data_packet(void** packet, __uint16_t block_number, void* data, int data_size) {
    int packet_size = PACKET_TYPE_SIZE + PACKET_BLOCK_NUMBER_SIZE + data_size;
    *packet = realloc(*packet, packet_size);
    bzero(*packet, packet_size);
    __uint16_t type = DTG_DATA;

    //add packet type
    memcpy(*packet, &type, PACKET_TYPE_SIZE);
    //add block number
    memcpy(*packet + PACKET_TYPE_SIZE, &block_number, PACKET_BLOCK_NUMBER_SIZE);
    //add data
    memcpy(*packet + PACKET_TYPE_SIZE + PACKET_BLOCK_NUMBER_SIZE, data, data_size);

    return packet_size;
}


int create_acknowledgment_packet(void** packet, uint16_t block_number) {
    int packet_size = PACKET_TYPE_SIZE + PACKET_BLOCK_NUMBER_SIZE;
    *packet = realloc(*packet, packet_size);
    uint16_t type = DTG_ACKNOWLEDGMENT;
    bzero(*packet, packet_size);

    memcpy(*packet, &type, PACKET_TYPE_SIZE);
    memcpy(*packet + PACKET_TYPE_SIZE, &block_number, PACKET_BLOCK_NUMBER_SIZE);

    return packet_size;
}


int create_error_packet(void** packet, int err_code, char* msg, int msg_size) {
    char zero_symb = '\0';
    int packet_size = PACKET_TYPE_SIZE + PACKET_BLOCK_NUMBER_SIZE + msg_size + sizeof(zero_symb);
    *packet = realloc(*packet, packet_size);
    uint16_t type = DTG_ERROR;

    bzero(*packet, packet_size);

    memcpy(*packet, &type, PACKET_TYPE_SIZE);
    memcpy(*packet + PACKET_TYPE_SIZE, &err_code, PACKET_BLOCK_NUMBER_SIZE);
    memcpy(*packet + PACKET_TYPE_SIZE + PACKET_BLOCK_NUMBER_SIZE, msg, msg_size);
    memcpy(*packet + PACKET_TYPE_SIZE + PACKET_BLOCK_NUMBER_SIZE + msg_size, &zero_symb, sizeof(zero_symb));

    return packet_size;
}