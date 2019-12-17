#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "user_choice.h"
#include "../constants.h"


void print_possible_user_actions_() {
    printf("Choose:\n");
    printf("%d) get list of topics.\n", GET_LIST_OF_TOPICS);
    printf("%d) get list of news.\n", GET_LIST_OF_NEWS);
    printf("%d) get news.\n", GET_NEWS);
    printf("%d) add topic.\n", ADD_TOPIC);
    printf("%d) add news.\n", ADD_NEWS);
    printf("%d) quit.\n", QUIT);
}


int get_user_choice() {
    char buffer[sizeof(int)+1];

    print_possible_user_actions_();
    fgets(buffer, sizeof(int), stdin);

    return atoi(buffer);
}


char* get_user_input(char* request_test) {
    char buffer[MAX_INPUT_USER_SIZE];
    bzero(buffer, MAX_INPUT_USER_SIZE);

    printf("%s", request_test);
    fflush(stdout);
    fgets(buffer, MAX_INPUT_USER_SIZE, stdin);
    buffer[strlen(buffer)-1] = '\0';

    return strdup(buffer);
}


//list of topics: topic, '\0' ... topic, '\0'
void print_list_of_topics(void* list_of_topics, int list_of_topics_size) {
    printf("------------\n");

    if (list_of_topics_size == 0) {
        printf("No topics.\n");
        printf("------------\n");
        return;
    }

    char* topic;
    for (int i = 1, offset = 0; offset < list_of_topics_size; i++) {
        topic = list_of_topics + offset;
        printf("%d) %s\n", i, topic);
        offset += (int) strlen(topic) + 1;
    }

    printf("------------\n");
}


//list of news: new headline, '\0' ... news headline, '\0'
void print_list_of_news(char* topic, void* list_of_news, int list_of_news_size) {
    char* news;
    printf("------------\n");
    printf("Topic: %s\n", topic);

    for (int i = 1, offset = 0; offset < list_of_news_size ; i++) {
        news = list_of_news + offset;
        printf("   %d) %s\n", i, news);
        offset += (int) strlen(news) + 1;
    }
    printf("------------\n");
}


void print_news(char* topic, char* news, char* text) {
    printf("------------\n");
    printf("Topic: %s\n", topic);
    printf("News header: %s\n", news);
    printf("Text: %s\n", text);
    printf("------------\n");
}