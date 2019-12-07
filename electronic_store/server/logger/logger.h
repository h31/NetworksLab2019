
#ifndef ELECTRONIC_STORE_SERVER_LOGGER_H
#define ELECTRONIC_STORE_SERVER_LOGGER_H

void log_product_action (const char* tag, int action, int product_count, char* product_name);

void log_client_action (const char* tag, int client_sockfd, char* msg);

void print_log (const char* tag, const char* message);

#endif //ELECTRONIC_STORE_SERVER_LOGGER_H
