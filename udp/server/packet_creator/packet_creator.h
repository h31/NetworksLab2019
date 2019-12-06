
#ifndef UDP_SERVER_LINUX_PACKET_CREATOR_H
#define UDP_SERVER_LINUX_PACKET_CREATOR_H

#include <stdint.h>

int create_data_packet(void** packet, __uint16_t block_number, void* data, int data_size);

int create_error_packet(void** packet, int err_code, char* msg, int msg_size);

int create_acknowledgment_packet(void** packet, uint16_t block_number);

#endif //UDP_SERVER_LINUX_PACKET_CREATOR_H
