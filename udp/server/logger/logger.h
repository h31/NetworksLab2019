#ifndef UDP_SERVER_LINUX_LOGGER_H
#define UDP_SERVER_LINUX_LOGGER_H

void print_data_packet_log(char* data_tag, in_addr_t cliaddr, int user_port, int packet_number, int packet_size);

void print_acknowledgment_log(char* ack_tag, in_addr_t cliaddr, int user_port, int packet_number);

void print_request_log(char* request_type, in_addr_t cliaddr, int user_port, char* file_name);

void print_error_log(int err_code, char* err_msg, in_addr_t cliaddr, int user_port);

void print_client_log( in_addr_t cliaddr, int user_port, char* action);

void print_log(const char* tag, const char* message);

#endif //UDP_SERVER_LINUX_LOGGER_H
