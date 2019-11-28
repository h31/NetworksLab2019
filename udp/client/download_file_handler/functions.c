#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <bits/types/FILE.h>

#include "../packet_creator/packet_creator.h"
#include "../packet_creator/packet_types.h"
#include "../packet_handler/packet_handler.h"
#include "../constants.h"
#include "../console_handler/console_handler.h"


int handle_error_packet_(char* packet, int packet_number, FILE** file) {
    console_print_error(packet_number, (char*) (packet + PACKET_BLOCK_NUMBER_SIZE + PACKET_TYPE_SIZE) );
    if (*file != NULL ) {
        fclose(*file);
    }

    return 0;
}


int handle_data_packet(char* packet, int packet_size, FILE** dst_file, char* dst_file_path, char* src_file_path) {
    uint16_t packet_number = ntohs(*(uint16_t*) (packet + sizeof(uint16_t)));

    //создание файла
    if (*dst_file == NULL) {
        *dst_file = fopen(dst_file_path, "a");
    }

    //запись данных
    fwrite(packet + PACKET_TYPE_SIZE + PACKET_BLOCK_NUMBER_SIZE,
           1,
           packet_size - PACKET_TYPE_SIZE - PACKET_BLOCK_NUMBER_SIZE,
           *dst_file);

    //подтверждение пакета
    send_acknowledgment_packet(packet_number);

    //если пришёл неполный пакет
    if (packet_size < MAX_PACKET_SIZE) {
        fclose(*dst_file);
        console_print_success_download_message(src_file_path);
        return -1;
    }

    return 1;
}


void download_file(char* file_path, char* dst_path) {
    char packet[MAX_PACKET_SIZE];
    struct sockaddr_in addr;
    uint16_t packet_number;
    uint16_t packet_type;
    FILE* file = NULL;
    int packet_size;
    int addr_len;

    bzero(packet, MAX_PACKET_SIZE);
    memset(&addr, 0, sizeof(addr));

    // отправка запроса на скачивание
    send_request_packet(DTG_READ_REQUEST, file_path);

    // получение пакетов с данными
    while ((packet_size = receive_packet(packet, &addr, &addr_len)) > 0) {
        packet_number = ntohs(*(uint16_t*) (packet + sizeof(uint16_t)));
        packet_type = ntohs(*(uint16_t*) packet);

        // проверка полученного пакета
        if ( (packet_type == DTG_DATA && handle_data_packet(packet, packet_size, &file, dst_path, file_path) == -1) ||
                (packet_type == DTG_ERROR && handle_error_packet_(packet, packet_number, &file) == 0) ) {
            return;
        }

        bzero(packet, MAX_PACKET_SIZE);
    }

    if (file == NULL) {
        console_print_error_message("Server is not responding");
    } else {
        fclose(file);
    }

}