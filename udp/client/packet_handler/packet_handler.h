
#ifndef UDP_CLIENT_LINUX_PACKET_HANDLER_H
#define UDP_CLIENT_LINUX_PACKET_HANDLER_H

#include <netinet/in.h>

void send_data_packet(int block_number, void* data, int data_size, struct sockaddr_in* addr, int addr_len);

void send_acknowledgment_packet(uint16_t block_number, struct sockaddr_in* addr, int addr_len);

void send_packet(void* packet, int packet_size, struct sockaddr_in* addr, int addr_len);

int receive_packet(void* buffer, struct sockaddr_in* addr, int* addr_len);

void send_request_packet(uint16_t request_type, char* file_path);

void packet_handler_init(char* host_name, uint16_t port_number);

#endif //UDP_CLIENT_LINUX_PACKET_HANDLER_H
