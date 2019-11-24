#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../constants.h"
#include "../packet_creator/packet_creator.h"
#include "../list_of_clients/user_modes.h"
#include "../list_of_clients/list_of_clients.h"


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
    int data_packet_size = create_data_packet(&data_packet, block_number, data, data_size);
    send_packet(data_packet, data_packet_size, &cliaddr, len);
    free(data_packet);
}


void send_acknowledgment_packet(int block_number, struct sockaddr_in cliaddr, int len) {
    void* acknowledgment_packet = NULL;
    int ack_packet_size = create_acknowledgment_packet(&acknowledgment_packet, block_number);
    send_packet(acknowledgment_packet, ack_packet_size, &cliaddr, len);
    free(acknowledgment_packet);
}


void send_error_packet(char* error_message, struct sockaddr_in cliaddr, int len) {
    void* err_packet = NULL;
    int err_packet_size = create_error_packet(&err_packet, 2, error_message, (int) strlen(error_message));
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
    //проверка существования клиента
    if (list_of_clients__client_exists(cliaddr) == 1) {
        send_error_packet("this user performs another operation", cliaddr, len);
        return;
    }

    //проверка существования файла
    char* file_name = (char*) (packet + PACKET_TYPE_SIZE);

    // проверка пути файла
    if (check_file_path_(file_name, strlen(file_name)) == -1) {
        send_error_packet("wrong file path", cliaddr, len);
        return;
    }

    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        send_error_packet("file does not exist", cliaddr, len);
        return;
    }

    //чтение данных
    void* data = malloc(MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE);
    bzero(data, MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE);
    int data_size = (int) fread(data, 1, MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE, file);

    //создание и отправка датаграммы
    send_data_packet(data, data_size, 1, cliaddr, len);
    free(data);

    //добавление пользователя в список, если послали полную датаграмму
    if (data_size == MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE) {
        list_of_clients__add_client(cliaddr, file, UMODE_WAIT_FOR_ACKNOWLEDGMENT, 1);
    }

}


void handle_write_request_packet(void* packet, struct sockaddr_in cliaddr, int len) {
    //проверка существования клиента
    if (list_of_clients__client_exists(cliaddr) == 1) {
        send_error_packet("this user performs another operation", cliaddr, len);
        return;
    }

    char* file_name = (char*) (packet + sizeof(uint16_t));

    // проверка пути файла
    if (check_file_path_(file_name, strlen(file_name)) == -1) {
        send_error_packet("wrong file path", cliaddr, len);
        return;
    }

    //проверка существования файла
    FILE* file = fopen(file_name, "r");
    if (file != NULL) {
        send_error_packet("file already exist", cliaddr, len);
        fclose(file);
        return;
    }

    //создаём файл для дозаписи
    file = fopen(file_name, "a");

    //добавляем пользователя в список
    list_of_clients__add_client(cliaddr, file, UMODE_WAIT_FOR_DATA, 1);

    //отправляем подтверждение
    send_acknowledgment_packet(0, cliaddr, len);
}


void handle_data_packet(void* packet, int packet_size, struct sockaddr_in cliaddr, int len) {
    //проверка существования клиента
    if (list_of_clients__client_exists(cliaddr) != 1) {
        send_error_packet("this user does not exist", cliaddr, len);
        return;
    }

    //проверка режима работы с пользователем
    if (list_of_clients__get_mode(cliaddr) != UMODE_WAIT_FOR_DATA) {
        send_error_packet("this user performs another operation", cliaddr, len);
        return;
    }

    // получение файла для записи
    FILE* file = list_of_clients__get_file(cliaddr);

    if (ferror(file) != 0) {
        send_error_packet("file does not exist", cliaddr, len);
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
        send_error_packet("not all data is recorded", cliaddr, len);
        //удаление клиента
        list_of_clients__remove_client(cliaddr);
        //закрытие файла
        fclose(file);
        return;
    }

    // отправка подтверждения
    uint16_t* packet_number = (uint16_t*) (packet + PACKET_TYPE_SIZE);
    send_acknowledgment_packet(*packet_number, cliaddr, len);

    // если был получен неполный пакет (последний)
    if (packet_size < MAX_PACKET_SIZE) {
        list_of_clients__remove_client(cliaddr);
        fclose(file);
        return;
    }
}


void handle_acknowledgment_packet(void* packet, struct sockaddr_in cliaddr, int len) {

    //проверка существования клиента
    if (list_of_clients__client_exists(cliaddr) != 1) {
        return;
    }

    //проверка режима работы с клиентом
    if (list_of_clients__get_mode(cliaddr) != UMODE_WAIT_FOR_ACKNOWLEDGMENT) {
        send_error_packet("this user performs another operation", cliaddr, len);
        return;
    }

    FILE* file = list_of_clients__get_file(cliaddr);

    //чтение следующей партии данных
    void* data = malloc(MAX_PACKET_SIZE - PACKET_TYPE_SIZE-PACKET_BLOCK_NUMBER_SIZE);
    int number_read = (int) fread(data, 1, MAX_PACKET_SIZE - PACKET_TYPE_SIZE-PACKET_BLOCK_NUMBER_SIZE, file);

    // отправление пакета с данными
    uint16_t* packet_number = (uint16_t*) (packet + PACKET_TYPE_SIZE);
    send_data_packet(data, number_read, (*(int *) packet_number) + 1, cliaddr, len);

    if (number_read < MAX_PACKET_SIZE-PACKET_TYPE_SIZE-PACKET_BLOCK_NUMBER_SIZE) {
        //если было прочитано меньше нужного, то это последний пакет => удаление клиента
        list_of_clients__remove_client(cliaddr);
    } else {
        //запись нового номера ожидаемого блока
        list_of_clients__set_block_number(cliaddr, (*(uint16_t *) packet_number) + 1);
    }
}