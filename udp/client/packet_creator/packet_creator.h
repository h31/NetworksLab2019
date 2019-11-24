
#ifndef UDP_CLIENT_LINUX_PACKET_CREATOR_H
#define UDP_CLIENT_LINUX_PACKET_CREATOR_H

#include <stdint.h>

int create_request_packet(void** packet, __uint16_t type, char* file_path, int file_path_size);

int create_data_packet(void** packet, __uint16_t block_number, void* data, int data_size);

int create_acknowledgment_packet(void** packet, uint16_t block_number);

#endif //UDP_CLIENT_LINUX_PACKET_CREATOR_H
