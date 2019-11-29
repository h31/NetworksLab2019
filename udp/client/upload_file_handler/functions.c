#include <bits/types/FILE.h>
#include <stdio.h>
#include <string.h>

#include "../console_handler/console_handler.h"
#include "../packet_creator/packet_creator.h"
#include "../packet_handler/packet_handler.h"
#include "../packet_creator/packet_types.h"
#include "../constants.h"


// проверка типа полученного пакета
int check_received_packet_(void* packet, int required_block_number) {
    uint16_t packet_number = ntohs(*(uint16_t*) (packet + sizeof(uint16_t)));
    uint16_t packet_type = ntohs(*(uint16_t*) packet);

    if (packet_type == DTG_ERROR) {
        console_print_error(packet_number, (char*) (packet + PACKET_TYPE_SIZE + PACKET_BLOCK_NUMBER_SIZE));
        return -1;
    }

    if (packet_type != DTG_ACKNOWLEDGMENT || packet_number != required_block_number) {
        fprintf(stdout, "Получили не подтверждение.\n");
        return -1;
    }

    return 0;
}


int upload_file_(FILE* file, struct sockaddr_in addr, int addr_len) {
    char buffer[MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE];
    char received_packet[MAX_PACKET_SIZE];
    int block_number = 0;
    int number_read;

    bzero(buffer, MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE);

    while ((number_read =
            (int) fread(buffer,1,MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE, file)) != 0) {
        block_number++;

        // создание и отпрака пакета данных
        send_data_packet(block_number, buffer, number_read);

        //ждём подтверждение
        receive_packet(received_packet, &addr, &addr_len);

        // получение полученного подтверждения
        if (check_received_packet_(received_packet, block_number) != 0) {
            return -1;
        }

        bzero(buffer, MAX_PACKET_SIZE - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE);
    }

    return 0;
}


void upload_file(char* file_path) {
    char received_packet[MAX_PACKET_SIZE];
    struct sockaddr_in addr;
    int addr_len;

    FILE* file = fopen(file_path, "r");

    if (file == NULL) {
        console_print_error_message("file not found");
        return;
    }

    // отправка запроса на запись
    send_request_packet(DTG_WRITE_REQUEST, file_path);

    // получение подтверждения отсервера
    receive_packet(received_packet, &addr, &addr_len);

    if (check_received_packet_(received_packet, 0) != 0) {
        return;
    };

    // загрузка файла на сервер
    if (upload_file_(file, addr, addr_len) != 0) {
        return;
    }

    console_print_success_upload_message(file_path);
}