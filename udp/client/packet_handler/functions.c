
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>

#include "../packet_creator/packet_creator.h"
#include "../constants.h"


int sockfd;
struct sockaddr_in servaddr;

static void fill_server_info_(char* host_name, int port_number) {
    struct hostent *server = gethostbyname(host_name);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_number);
    bcopy(server->h_addr, (char *) &servaddr.sin_addr.s_addr, (size_t) server->h_length);
}


static void create_socket_() {
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("ERROR: socket creation failed.\n");
        exit(EXIT_FAILURE);
    }
}


void packet_handler_init(char* host_name, uint16_t port_number) {
    create_socket_();
    fill_server_info_(host_name, port_number);
}


void send_packet(void* packet, int packet_size,  struct sockaddr_in* addr, int addr_len) {
    sendto(sockfd, packet, packet_size,
           MSG_CONFIRM, (const struct sockaddr *) addr,
           addr_len);
}


int receive_packet(void* buffer, struct sockaddr_in* addr, int* addr_len) {
    return recvfrom(sockfd, buffer, MAX_PACKET_SIZE,MSG_WAITALL, (struct sockaddr*) addr, (socklen_t *) addr_len);
}


void send_request_packet(uint16_t request_type, char* file_path) {
    void* request_packet = NULL;
    int request_packet_size =create_request_packet(&request_packet, htons(request_type), file_path,
            strnlen(file_path, MAX_FILE_PATH_SIZE));
    send_packet(request_packet, request_packet_size, &servaddr, sizeof(servaddr));
    free(request_packet);
}


void send_data_packet(int block_number, void* data, int data_size, struct sockaddr_in* addr, int addr_len) {
    void* data_packet = NULL;
    int data_packet_size = create_data_packet(&data_packet, htons(block_number), data, data_size);
    send_packet(data_packet, data_packet_size, addr == NULL ? &servaddr : addr,
            addr == NULL ? (int) sizeof(servaddr) : addr_len);
    free(data_packet);
}


void send_acknowledgment_packet(uint16_t block_number, struct sockaddr_in* addr, int addr_len) {
    void *acknowledgment_packet = NULL;
    int ack_packet_size = create_acknowledgment_packet(&acknowledgment_packet, htons(block_number) );
    send_packet(acknowledgment_packet, ack_packet_size, addr == NULL ? &servaddr : addr,
            addr == NULL ? (int) sizeof(addr_len) : addr_len);
    free(acknowledgment_packet);
}