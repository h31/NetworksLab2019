#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "packet_types.h"
#include "../constants.h"


int create_request_packet(void** packet, __uint16_t type, char* file_path, int file_path_size) {
    char zero_sym = '\0';
    char* mode = "octet";
    int packet_size = PACKET_TYPE_SIZE + file_path_size + sizeof(zero_sym) + strlen(mode) + sizeof(zero_sym);
    *packet = realloc(*packet, packet_size);
    bzero(*packet, packet_size);

    //add packet type
    memcpy(*packet, &type, PACKET_TYPE_SIZE);
    //add file name
    memcpy(*packet + PACKET_TYPE_SIZE, (void*) file_path, file_path_size);
    //add '\0'
    memcpy(*packet + PACKET_TYPE_SIZE + file_path_size, &zero_sym, sizeof(zero_sym));
    //add mode
    memcpy(*packet + PACKET_TYPE_SIZE + file_path_size + sizeof(zero_sym), mode, strlen(mode));
    //add '\0'
    memcpy(*packet + PACKET_TYPE_SIZE + file_path_size + sizeof(zero_sym) + strlen(mode),
            &zero_sym,
            sizeof(zero_sym));

    return packet_size;
}


int create_data_packet(void** packet, __uint16_t block_number, void* data, int data_size) {
    int packet_size = PACKET_TYPE_SIZE + PACKET_BLOCK_NUMBER_SIZE + data_size;
    *packet = realloc(*packet, packet_size);
    bzero(*packet, packet_size);
    __uint16_t type = htons(DTG_DATA);

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
    uint16_t type = htons(DTG_ACKNOWLEDGMENT);

    bzero(*packet, packet_size);

    memcpy(*packet, &type, PACKET_TYPE_SIZE);
    memcpy(*packet + PACKET_TYPE_SIZE, &block_number, PACKET_BLOCK_NUMBER_SIZE);

    return packet_size;
}