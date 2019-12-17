#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "../packet_manager/acknowledgment_types.h"
#include "../window_manager/window_manager.h"
#include "../packet_manager/packet_manager.h"
#include "../packet_manager/packet_types.h"
#include "../packet_manager/error_types.h"
#include "../constants.h"

static void handle_error_packet_(const uint16_t* err_type, const char* err_msg) {
    switch (*err_type) {
        case LACK_OF_NEWS_TOPIC:
            printf("ERROR lack of news topic; error type : %d; error message: %s.\n", *err_type, err_msg);
            break;
        case NO_NEWS_WITH_THIS_HEADLINE:
            printf("ERROR no news with this headline; error type : %d; error message: %s.\n", *err_type, err_msg);
            break;
        case ADDING_EXISTING_NEWS_TOPIC:
            printf("ERROR adding existing news topic; error type : %d; error message: %s.\n", *err_type, err_msg);
            break;
        case ADDITION_OF_NEWS_WITH_EXISTING_HEADER:
            printf("ERROR addition of news with existing header; error type : %d; error message: %s.\n",
                    *err_type, err_msg);
            break;
    }
}


static void handle_acknowledgment_packet_(const uint16_t* ack_type) {
    switch (*ack_type){
        case TOPIC_ADDED:
            printf("Topic successfully added.\n");
            break;
        case NEWS_ADDED:
            printf("News successfully added.\n");
            break;
    }
}


static void handle_received_packet_(char * received_packet, int received_packet_length) {
    uint16_t* received_packet_type;
    char* news_title;
    char* topic;

    received_packet_type = (uint16_t*) (received_packet + SIZE_OF_PACKET_LENGTH);
    switch (*received_packet_type) {
        case ERROR_PACKET:
            handle_error_packet_((uint16_t*) (received_packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE),
                    received_packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + SIZE_OF_ERR_TYPE);
            break;

        case ACKNOWLEDGMENT_PACKET:
            handle_acknowledgment_packet_(
                    (uint16_t*) (received_packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE)
                    );
            break;

        case LIST_OF_NEWS_PACKET:
            topic = received_packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE;
            print_list_of_news(topic,
                    received_packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE +
                    strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE,
                    received_packet_length - SIZE_OF_PACKET_LENGTH - SIZE_OF_PACKET_TYPE -
                    (int) strlen(topic) - SIZE_OF_PACKET_ZERO_BYTE);
            break;

        case LIST_OF_TOPICS_PACKET:
            print_list_of_topics(received_packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE,
                                 received_packet_length - SIZE_OF_PACKET_LENGTH - SIZE_OF_PACKET_TYPE
            );
            break;

        case NEWS_PACKET:
            topic = received_packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE;
            news_title = topic + strlen(topic) + SIZE_OF_PACKET_ZERO_BYTE;
            print_news(topic,
                       news_title,
                       received_packet + SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + strlen(topic) +
                       SIZE_OF_PACKET_ZERO_BYTE + strlen(news_title) + SIZE_OF_PACKET_ZERO_BYTE
            );
            break;
    }
}


int handle_get_list_of_news_user_choice(int sockfd) {
    int received_packet_length;
    void* received_packet;

    char* topic = get_user_input("Enter topic: ");

    if (send_list_of_news_request(sockfd, topic) <= 0) {
        free(topic);
        return -1;
    }

    received_packet = receive_packet(sockfd, &received_packet_length);

    if (received_packet_length == 0) {
        free(topic);
        free(received_packet);
        return -1;
    }

    handle_received_packet_(received_packet, received_packet_length);

    free(topic);
    free(received_packet);

    return 1;
}


int handle_get_news_user_choice(int sockfd) {
    int received_packet_length;

    char* topic = get_user_input("Enter topic: ");

    char* news_title = get_user_input("Enter news title: ");

    // отправить запрос
    if (send_get_news_request(sockfd, topic, news_title) <= 0) {
        free(topic);
        free(news_title);
        return -1;
    }

    void* received_packet = receive_packet(sockfd, &received_packet_length);

    if (received_packet_length == 0) {
        free(topic);
        free(news_title);
        free(received_packet);
        return -1;
    }

    handle_received_packet_(received_packet,received_packet_length);

    free(topic);
    free(news_title);
    free(received_packet);

    return 1;
}


int handle_add_topic_user_choice(int sockfd) {
    int received_packet_length;

    char* topic = get_user_input("Enter topic: ");

    if (send_add_topic_request(sockfd, topic) <= 0) {
        free(topic);
        return -1;
    }

    void* received_packet = receive_packet(sockfd, &received_packet_length);

    if (received_packet_length == 0) {
        free(topic);
        free(received_packet);
        return -1;
    }

    handle_received_packet_(received_packet, received_packet_length);

    free(topic);
    free(received_packet);

    return 1;
}



int handle_get_list_of_topics_user_choice(int sockfd) {
    int received_packet_length = 0;

    if (send_list_of_topics_request(sockfd) <= 0) {
        return -1;
    }

    void* received_packet = receive_packet(sockfd, &received_packet_length);

    if (received_packet_length == 0) {
        free(received_packet);
        return -1;
    }

    handle_received_packet_( (char*) received_packet, received_packet_length);

    free(received_packet);

    return 1;
}


int handle_add_news_user_choice(int sockfd) {
    int received_packet_length;

    char* topic = get_user_input("Enter topic:");
    char* news_title = get_user_input("Enter news title:");
    char* news_text = get_user_input("Enter text:");

    if (send_news_packet(sockfd, topic, news_title, news_text) <= 0) {
        free(topic);
        free(news_title);
        free(news_text);
        return -1;
    }

    void* received_packet = receive_packet(sockfd, &received_packet_length);

    if (received_packet_length == 0) {
        free(received_packet);
        free(topic);
        free(news_title);
        free(news_text);
        return -1;
    }

    handle_received_packet_(received_packet, received_packet_length);

    free(received_packet);
    free(topic);
    free(news_title);
    free(news_text);

    return 1;
}