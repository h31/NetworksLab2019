#include <stdio.h>
#include <time.h>
#include <netinet/in.h>


static time_t get_time_() {
    time_t cur_time;
    time(&cur_time);
    return cur_time;
}


void print_log(const char* tag, const char* message) {
    time_t now = get_time_();
    printf("%s [%s]: %s\n", ctime(&now), tag, message);
}


void print_request_log(char* request_type, in_addr_t cliaddr, int user_port, char* file_name) {
    time_t now = get_time_();
    printf("%s [%s] user address: %d, user port: %d, file name: %s\n", ctime(&now), request_type, cliaddr, user_port, file_name);
}


void print_data_packet_log( char* data_tag, in_addr_t cliaddr, int user_port, int packet_number, int packet_size) {
    time_t now = get_time_();
    printf("%s [%s] user address: %d, user port: %d, packet number: %d, packet size: %d\n", ctime(&now), data_tag, cliaddr, user_port, packet_number, packet_size);
}


void print_acknowledgment_log(char* ack_tag, in_addr_t cliaddr, int user_port, int packet_number) {
    time_t now = get_time_();
    printf("%s [%s] user address: %d, user port: %d, packet number: %d\n", ctime(&now), ack_tag, cliaddr, user_port, packet_number);
}


void print_client_log( in_addr_t cliaddr, int user_port, char* action) {
    time_t now = get_time_();
    printf("%s [CLIENT ACTION] user address: %d, user port: %d, action: %s\n", ctime(&now), cliaddr, user_port, action);
}


void print_error_log(int err_code, char* err_msg, in_addr_t cliaddr, int user_port) {
    time_t now = get_time_();
    printf("%s [ERROR] user address: %d, user port: %d, error code: %d, error message: %s\n", ctime(&now), cliaddr, user_port, err_code, err_msg);
}
