
#ifndef ELECTRONIC_STORE_PACKET_HANDLER_H
#define ELECTRONIC_STORE_PACKET_HANDLER_H

void handle_add_product_packet(int client_sockfd, void* packet);

void handle_buy_product_packet(int client_sockfd, void* packet);

#endif //ELECTRONIC_STORE_PACKET_HANDLER_H
