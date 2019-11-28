#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

#include "packet_types.h"
#include "constants.h"
#include "./list_of_clients/list_of_clients.h"
#include "./packet_creator/packet_creator.h"
#include "./packet_handler/packet_handler.h"


void check_number_of_args_(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s port\n", argv[0]);
        exit(0);
    }
}


int main(int argc, char* argv[]) {
    char packet[MAX_PACKET_SIZE];
    struct sockaddr_in cliaddr;
    uint16_t packet_type;
    int packet_size;
    int cliaddr_len;

    check_number_of_args_(argc, argv);

    const uint16_t port = (uint16_t) atoi(argv[1]);

    packet_handler_init(port);
    list_of_clients__init();

    while (1) {
        bzero(packet, MAX_PACKET_SIZE);
        bzero(&cliaddr, sizeof(cliaddr));

        packet_size = receive_packet(packet, &cliaddr, &cliaddr_len);

        if (packet_size == -1 && errno != EAGAIN) {
            fprintf(stdout, "Error: code: %d; msg: packet size = -1\n", errno);
            continue;
        }

        if (packet_size == 0) {
            continue;
        }

        packet_type = ntohs(*(uint16_t*) packet);
        if (packet_type == DTG_READ_REQUEST) {
            handle_read_request_packet(packet, cliaddr, cliaddr_len);

        } else if (packet_type == DTG_WRITE_REQUEST) {
            handle_write_request_packet(packet, cliaddr, cliaddr_len);

        } else if (packet_type == DTG_DATA) {
            handle_data_packet(packet, packet_size, cliaddr, cliaddr_len);

        } else if (packet_type == DTG_ACKNOWLEDGMENT) {
            handle_acknowledgment_packet(packet, cliaddr, cliaddr_len);
        }

    }

}