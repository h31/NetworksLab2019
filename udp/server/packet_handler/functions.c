#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../constants.h"
#include "../packet_creator/packet_creator.h"
#include "../list_of_clients/user_modes.h"
#include "../list_of_clients/list_of_clients.h"
#include "../logger/logger.h"
#include "../logger/types.h"
#include "./error_codes.h"


int sockfd;
struct sockaddr_in servaddr;


static void make_socket_reusable_() {
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        fprintf(stderr, "ERROR: setsockopt(SO_REUSEADDR) failed");
    }
}


static void bind_socket_() {
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
              sizeof(servaddr)) < 0 ){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}


static void create_socket_() {
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("ERROR: socket creation failed.\n");
        exit(EXIT_FAILURE);
    }
}


static void fill_server_info_(int port_number) {
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_number);
    servaddr.sin_addr.s_addr = INADDR_ANY;
}


void packet_handler_init(uint16_t port_number) {
    create_socket_();
    fill_server_info_(port_number);
    bind_socket_();
    make_socket_reusable_();
}


void send_packet(void* packet, int packet_size, struct sockaddr_in* cliaddr, int cliaddr_len) {
    sendto(sockfd, packet, packet_size,MSG_CONFIRM, (const struct sockaddr *) cliaddr, cliaddr_len);
}


int receive_packet(void* buffer, struct sockaddr_in* cliaddr, int* cliaddr_len) {
    return (int) recvfrom(sockfd, buffer, (size_t) MAX_PACKET_SIZE, MSG_DONTWAIT, cliaddr, cliaddr_len);
}


void send_data_packet(void* data, int data_size, int block_number, struct sockaddr_in cliaddr, int len) {
    void* data_packet = NULL;
    int data_packet_size = create_data_packet(&data_packet, htons(block_number), data, data_size);
    send_packet(data_packet, data_packet_size, &cliaddr, len);
    free(data_packet);
}


void send_acknowledgment_packet(int block_number, struct sockaddr_in cliaddr, int len) {
    void* acknowledgment_packet = NULL;
    int ack_packet_size = create_acknowledgment_packet(&acknowledgment_packet, htons(block_number));
    send_packet(acknowledgment_packet, ack_packet_size, &cliaddr, len);
    free(acknowledgment_packet);
}


void send_error_packet(int err_code, char* error_message, struct sockaddr_in cliaddr, int len) {
    void* err_packet = NULL;
    int err_packet_size = create_error_packet(&err_packet, htons(err_code), error_message, (int) strlen(error_message));
    send_packet(err_packet, err_packet_size, &cliaddr, len);
    free(err_packet);
}


static int check_file_path_(const char* file_path, const int file_path_size) {
    int i = 1;

    while (i < file_path_size) {
        if (file_path[i] == '/' && file_path[i-1] == '.') {
            return -1;
        }
        i++;
    }

    return 1;
}


void handle_read_request_packet(void* packet, struct sockaddr_in cliaddr, int len) {

    print_request_log(RECEIVE_READ_REQUEST, cliaddr.sin_addr.s_addr, cliaddr.sin_port, (char*) (packet + PACKET_TYPE_SIZE));

    //проверка существования клиента
    if (list_of_clients__client_exists(cliaddr) == 1) {
        send_error_packet(ACCESS_VIOLATION,"this user performs another operation", cliaddr, len);
        print_error_log(ACCESS_VIOLATION, "this user performs another operation", cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        return;
    }

    //проверка существования файла
    char* file_name = (char*) (packet + PACKET_TYPE_SIZE);

    // проверка пути файла
    if (check_file_path_(file_name, strlen(file_name)) == -1) {
        send_error_packet(FILE_NOT_FOUND, "wrong file path", cliaddr, len);
        print_error_log(FILE_NOT_FOUND, "wrong file path", cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        return;
    }

    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        send_error_packet(FILE_NOT_FOUND, "file does not exist", cliaddr, len);
        print_error_log(FILE_NOT_FOUND, "file does not exist", cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        return;
    }

    //чтение данных
    void* data = malloc(MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE);
    bzero(data, MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE);
    int data_size = (int) fread(data, 1, MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE, file);

    //создание и отправка датаграммы
    send_data_packet(data, data_size, 1, cliaddr, len);
    print_data_packet_log(SEND_DATA_PACKET, cliaddr.sin_addr.s_addr, cliaddr.sin_port, 1, data_size);
    free(data);

    //добавление пользователя в список, если послали полную датаграмму
    if (data_size == MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE) {
        list_of_clients__add_client(cliaddr, file, UMODE_WAIT_FOR_ACKNOWLEDGMENT, 1);
        print_client_log(cliaddr.sin_addr.s_addr, cliaddr.sin_port, "client has been added to list");
    }

}


void handle_write_request_packet(void* packet, struct sockaddr_in cliaddr, int len) {

    print_request_log(RECEIVE_WRITE_REQUEST, cliaddr.sin_addr.s_addr, cliaddr.sin_port, (char*) (packet + PACKET_TYPE_SIZE));

    //проверка существования клиента
    if (list_of_clients__client_exists(cliaddr) == 1) {
        send_error_packet(ACCESS_VIOLATION, "this user performs another operation", cliaddr, len);
        print_error_log(ACCESS_VIOLATION, "this user performs another operation", cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        return;
    }

    char* file_name = (char*) (packet + PACKET_TYPE_SIZE);

    // проверка пути файла
    if (check_file_path_(file_name, strlen(file_name)) == -1) {
        send_error_packet(FILE_NOT_FOUND, "wrong file path", cliaddr, len);
        print_error_log(FILE_NOT_FOUND, "wrong file path", cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        return;
    }

    //проверка существования файла
    FILE* file = fopen(file_name, "r");
    if (file != NULL) {
        send_error_packet(FILE_ALREADY_EXISTS, "file already exist", cliaddr, len);
        print_error_log(FILE_ALREADY_EXISTS, "file already exist", cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        fclose(file);
        return;
    }

    //создаём файл для дозаписи
    file = fopen(file_name, "a");

    //добавляем пользователя в список
    list_of_clients__add_client(cliaddr, file, UMODE_WAIT_FOR_DATA, 1);
    print_client_log(cliaddr.sin_addr.s_addr, cliaddr.sin_port, "client has been added to list");

    //отправляем подтверждение
    send_acknowledgment_packet(0, cliaddr, len);
    print_acknowledgment_log(SEND_ACKNOWLEDGMENT_PACKET, cliaddr.sin_addr.s_addr, cliaddr.sin_port, 0);
}


void handle_data_packet(void* packet, int packet_size, struct sockaddr_in cliaddr, int len) {

    print_data_packet_log(RECEIVE_DATA_PACKET, cliaddr.sin_addr.s_addr, cliaddr.sin_port, ntohs( *(uint16_t*) (packet + PACKET_TYPE_SIZE)), packet_size);

    //проверка существования клиента
    if (list_of_clients__client_exists(cliaddr) != 1) {
        send_error_packet(NO_SUCH_USER, "this user does not exist", cliaddr, len);
        print_error_log(NO_SUCH_USER, "this user does not exist", cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        return;
    }

    //проверка режима работы с пользователем
    if (list_of_clients__get_mode(cliaddr) != UMODE_WAIT_FOR_DATA) {
        send_error_packet(ACCESS_VIOLATION, "this user performs another operation", cliaddr, len);
        print_error_log(ACCESS_VIOLATION, "this user performs another operation", cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        return;
    }

    // получение файла для записи
    FILE* file = list_of_clients__get_file(cliaddr);

    if (ferror(file) != 0) {
        send_error_packet(FILE_NOT_FOUND, "file does not exist", cliaddr, len);
        print_error_log(FILE_NOT_FOUND, "file does not exist", cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        //удаление клиента
        list_of_clients__remove_client(cliaddr);
        //закрытие файла
        fclose(file);
        return;
    }

    //запись полученных данных
    void* data = packet + PACKET_TYPE_SIZE + PACKET_BLOCK_NUMBER_SIZE;
    int data_size = packet_size - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE;
    int number_written = (int) fwrite(data, 1, data_size, file);

    //проверка записи
    if (number_written != data_size) {
        send_error_packet(DISK_FULL_OR_ALLOCATION_EXCEEDED, "not all data is recorded", cliaddr, len);
        print_error_log(DISK_FULL_OR_ALLOCATION_EXCEEDED, "not all data is recorded", cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        //удаление клиента
        list_of_clients__remove_client(cliaddr);
        //закрытие файла
        fclose(file);
        return;
    }

    // отправка подтверждения
    uint16_t packet_number = ntohs( *(uint16_t*) (packet + PACKET_TYPE_SIZE));
    send_acknowledgment_packet(packet_number, cliaddr, len);
    print_acknowledgment_log(SEND_ACKNOWLEDGMENT_PACKET, cliaddr.sin_addr.s_addr, cliaddr.sin_port, packet_number);

    // если был получен неполный пакет (последний)
    if (packet_size < MAX_PACKET_SIZE) {
        list_of_clients__remove_client(cliaddr);
        print_client_log(cliaddr.sin_addr.s_addr, cliaddr.sin_port, "client has been removed from list");
        fclose(file);
        return;
    }
}


void handle_acknowledgment_packet(void* packet, struct sockaddr_in cliaddr, int len) {

    print_acknowledgment_log(RECEIVE_ACKNOWLEDGMENT_PACKET, cliaddr.sin_addr.s_addr, cliaddr.sin_port, ntohs( *(uint16_t*) (packet + PACKET_TYPE_SIZE)));

    //проверка существования клиента
    if (list_of_clients__client_exists(cliaddr) != 1) {
        return;
    }

    //проверка режима работы с клиентом
    if (list_of_clients__get_mode(cliaddr) != UMODE_WAIT_FOR_ACKNOWLEDGMENT) {
        send_error_packet(ACCESS_VIOLATION, "this user performs another operation", cliaddr, len);
        print_error_log(ACCESS_VIOLATION, "this user performs another operation", cliaddr.sin_addr.s_addr, cliaddr.sin_port);
        return;
    }

    FILE* file = list_of_clients__get_file(cliaddr);

    //чтение следующей партии данных
    void* data = malloc(MAX_PACKET_SIZE - PACKET_TYPE_SIZE-PACKET_BLOCK_NUMBER_SIZE);
    int number_read = (int) fread(data, 1, MAX_PACKET_SIZE - PACKET_TYPE_SIZE-PACKET_BLOCK_NUMBER_SIZE, file);

    // отправление пакета с данными
    uint16_t packet_number = ntohs( *(uint16_t*) (packet + PACKET_TYPE_SIZE));
    send_data_packet(data, number_read, packet_number + 1, cliaddr, len);
    print_data_packet_log(SEND_DATA_PACKET, cliaddr.sin_addr.s_addr, cliaddr.sin_port, packet_number, number_read);

    if (number_read < MAX_PACKET_SIZE-PACKET_TYPE_SIZE-PACKET_BLOCK_NUMBER_SIZE) {
        //если было прочитано меньше нужного, то это последний пакет => удаление клиента
        list_of_clients__remove_client(cliaddr);
        print_client_log(cliaddr.sin_addr.s_addr, cliaddr.sin_port, "client has been removed from list");
    } else {
        //запись нового номера ожидаемого блока
        list_of_clients__set_block_number(cliaddr, htons(packet_number + 1) );
    }
}