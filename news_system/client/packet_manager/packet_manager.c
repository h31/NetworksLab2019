
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../constants.h"
#include "./packet_types.h"


void* create_list_of_topics_request_packet_(uint32_t* packet_length) {
    *packet_length = SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE;
    uint16_t packet_type = GET_LIST_OF_TOPICS_PACKET;
    void* packet = malloc(*packet_length);

    memcpy(packet, packet_length, SIZE_OF_PACKET_LENGTH);
    memcpy( (char*) packet + SIZE_OF_PACKET_LENGTH, &packet_type, SIZE_OF_PACKET_TYPE);

    return packet;
}


int send_list_of_topics_request(int sockfd) {
    int number_writen;
    uint32_t packet_length;
    void* packet = create_list_of_topics_request_packet_(&packet_length);

    number_writen = write(sockfd, packet, packet_length);
    free(packet);

    return number_writen;
}


void* create_list_of_news_request_packet_(uint32_t* packet_length, char* topic) {
    *packet_length = SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE;
    uint16_t packet_type = GET_LIST_OF_NEWS_PACKET;
    char* packet = (char*) malloc(*packet_length);
    char zero_byte = '\0';

    memcpy(packet, packet_length, SIZE_OF_PACKET_LENGTH);
    memcpy(packet + SIZE_OF_PACKET_LENGTH, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE, topic, strlen(topic));
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic),
           &zero_byte,
           SIZE_OF_PACKET_ZERO_BYTE
    );

    return packet;
}


int send_list_of_news_request(int sockfd, char* topic) {
    int number_written;
    uint32_t packet_length;
    void* packet = create_list_of_news_request_packet_(&packet_length, topic);

    number_written = write(sockfd, packet, packet_length);
    free(packet);

    return number_written;
}


void* create_get_news_request_packet_(uint32_t *packet_length, char *topic, char *news_topic) {
    *packet_length = SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE +
                     strlen(news_topic) + SIZE_OF_PACKET_ZERO_BYTE;
    uint16_t packet_type = GET_NEWS_PACKET;
    char *packet = (char*) malloc(*packet_length);
    char zero_byte = '\0';

    memcpy(packet, packet_length, SIZE_OF_PACKET_LENGTH);
    memcpy(packet + SIZE_OF_PACKET_LENGTH, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE, topic, strlen(topic));
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic), &zero_byte, SIZE_OF_PACKET_ZERO_BYTE);
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE,
           news_topic,
           strlen(news_topic)
    );
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE +
           strlen(news_topic),
           &zero_byte,
           SIZE_OF_PACKET_ZERO_BYTE
    );

    return packet;
}


int send_get_news_request(int sockfd, char* topic, char* news_topic) {
    int number_written;
    uint32_t packet_length;
    void* packet = create_get_news_request_packet_(&packet_length, topic, news_topic);

    number_written = write(sockfd, packet, packet_length);
    free(packet);

    return number_written;
}


void* create_add_topic_packet_(uint32_t* packet_length, char* topic) {
    *packet_length = SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE;
    uint16_t packet_type = ADD_TOPIC_PACKET;
    char* packet = malloc(*packet_length);
    char zero_byte = '\0';

    memcpy(packet, packet_length, SIZE_OF_PACKET_LENGTH);
    memcpy(packet + SIZE_OF_PACKET_LENGTH, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE, topic, strlen(topic));
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic), &zero_byte, SIZE_OF_PACKET_ZERO_BYTE);

    return packet;
}


int send_add_topic_request(int sockfd, char* topic) {
    int number_writen;
    uint32_t packet_length;
    void* packet = create_add_topic_packet_(&packet_length, topic);

    number_writen = write(sockfd, packet, packet_length);
    free(packet);

    return number_writen;
}


void* create_news_packet_(uint32_t* packet_length, char* topic, char* news_title, char* text) {
    *packet_length = SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE +
            strlen(news_title) + SIZE_OF_PACKET_ZERO_BYTE + strlen(text) + SIZE_OF_PACKET_ZERO_BYTE;
    uint16_t packet_type = ADD_NEWS_PACKET;
    char zero_byte = '\0';
    char* packet = (char*) malloc(*packet_length);

    memcpy(packet, packet_length, SIZE_OF_PACKET_LENGTH);
    memcpy(packet + SIZE_OF_PACKET_LENGTH, &packet_type, SIZE_OF_PACKET_TYPE);
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE, topic, strlen(topic));
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic), &zero_byte, SIZE_OF_PACKET_ZERO_BYTE);
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE,
           news_title,
           strlen(news_title)
    );
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE +
           strlen(news_title),
           &zero_byte,
           SIZE_OF_PACKET_ZERO_BYTE
    );
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE +
           strlen(news_title) + SIZE_OF_PACKET_ZERO_BYTE,
           text,
           strlen(text)
    );
    memcpy(packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE +
           strlen(news_title) + SIZE_OF_PACKET_ZERO_BYTE + strlen(text),
           &zero_byte,
           SIZE_OF_PACKET_ZERO_BYTE
    );

    return packet;
}


// отправление ппакета с текстом новости по определённой теме
int send_news_packet(int sockfd, char* topic, char* news_title,  char* news_text) {
    uint32_t packet_length;

    void* packet = create_news_packet_(&packet_length, topic, news_title, news_text);
    int number_written = write(sockfd, packet, packet_length);
    free(packet);

    return number_written;
}


static int readn_(int sockfd, void *dst, size_t len) {
    int total_number_read = 0;
    int local_number_read;

    while (len > 0) {
        local_number_read = read(sockfd, (char*) dst + total_number_read, len);

        if (local_number_read == 0) {
            return total_number_read;
        }

        if (local_number_read < 0) {
            return local_number_read;
        }

        total_number_read += local_number_read;
        len -= local_number_read;
    }

    return total_number_read;
}


void* receive_packet(int sockfd, int* packet_length) {
    void* packet;

    readn_(sockfd, packet_length, SIZE_OF_PACKET_LENGTH);
    packet = malloc(*packet_length);
    memcpy(packet, packet_length, SIZE_OF_PACKET_LENGTH);
    readn_(sockfd, (char*)packet + SIZE_OF_PACKET_LENGTH, *packet_length - SIZE_OF_PACKET_LENGTH);

    return packet;
}