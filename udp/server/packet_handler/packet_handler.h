
#ifndef UDP_SERVER_LINUX_PACKET_HANDLER_H
#define UDP_SERVER_LINUX_PACKET_HANDLER_H

#include <stdint.h>

void send_data_packet(void* data, int data_size, int block_number, struct sockaddr_in cliaddr, int len);

void send_error_packet(int err_code, char* error_message, struct sockaddr_in cliaddr, int len);

void send_packet(void* packet, int packet_size, struct sockaddr_in* cliaddr, int cliaddr_len);

void handle_data_packet(void* packet, int packet_size, struct sockaddr_in cliaddr, int len);

void send_acknowledgment_packet(int block_number, struct sockaddr_in cliaddr, int len);

void handle_acknowledgment_packet(void* packet, struct sockaddr_in cliaddr, int len);

void handle_write_request_packet(void* packet, struct sockaddr_in cliaddr, int len);

void handle_read_request_packet(void* packet, struct sockaddr_in cliaddr, int len);

int receive_packet(void* buffer, struct sockaddr_in* cliaddr, int* cliaddr_len);

void packet_handler_init(uint16_t port_number);

#endif //UDP_SERVER_LINUX_PACKET_HANDLER_H
