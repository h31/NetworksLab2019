#include <stdio.h>
#include <time.h>
#include "./actions_with_product.h"

static time_t get_time_() {
    time_t cur_time;
    time(&cur_time);
    return cur_time;
}


void print_log (const char* tag, const char* message) {
    time_t now = get_time_();
    printf("%s [%s]: %s\n", ctime(&now), tag, message);
}


void log_client_action (const char* tag, int client_sockfd, char* msg) {
    time_t now = get_time_();
    printf("%s [%s]: %s. Socket: %d.\n", ctime(&now), tag, msg, client_sockfd);
}


void log_product_action (const char* tag, int action, int product_count, char* product_name) {
    time_t now = get_time_();
    printf("%s [%s]: %s %d items of product '%s'.\n",
            ctime(&now), tag, action == ADD_PRODUCT ? "add" : "buy", product_count, product_name);
}


