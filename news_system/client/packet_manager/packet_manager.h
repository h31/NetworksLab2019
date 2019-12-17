
#ifndef NEW_SYSTEM_CLIENT_PACKET_MANAGER_H
#define NEW_SYSTEM_CLIENT_PACKET_MANAGER_H

int send_list_of_news_request(int sockfd, char* topic);

int send_list_of_topics_request(int sockfd);

int send_get_news_request(int sockfd, char* topic, char* news_topic);

int send_add_topic_request(int sockfd, char* topic);

void* receive_packet(int sockfd, int* packet_length);

int send_news_packet(int sockfd, char* topic, char* news_title, char* news_text);

#endif //NEW_SYSTEM_CLIENT_PACKET_MANAGER_H
