#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include "./threads/console_listening_thread/thread.h"
#include "./list_of_products/list_of_products.h"
#include "./threads/accepting_thread/thread.h"
#include "./list_of_clients/list_of_clients.h"


void checkArguments(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage %s port \n", argv[0]);
        exit(0);
    };
}

int main(int argc, char* argv[]) {
    int initial_socket;

    checkArguments(argc, argv);

    const uint16_t port_number = (uint16_t)  atoi(argv[1]);

    // открытие начального сокета
    initial_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (initial_socket < 0) {
        fprintf(stderr, "ERROR opening init socket\n");
        exit(1);
    }


    init_list_of_clients_mutex();
    init_list_of_products_mutex();


    pthread_t accepting_thread = create_accepting_thread(port_number, initial_socket);
    pthread_t console_listening_thread = create_user_listening_thread(&initial_socket);


    pthread_join(accepting_thread, NULL);
    pthread_join(console_listening_thread, NULL);


    list_of_clients_remove_all();
    list_of_products_remove_all();

}