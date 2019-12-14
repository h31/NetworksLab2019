
#ifndef NEW_SYSTEM_CLIENT_WINDOW_MANAGER_H
#define NEW_SYSTEM_CLIENT_WINDOW_MANAGER_H

int get_user_choice();

void print_list_of_topics(void* list_of_topics, int list_of_topics_size);

char* get_user_input(char* request_test);

void print_list_of_news(char* topic, void* list_of_news, int list_of_news_length);

void print_news(char* topic, char* news, char* text);

#endif //NEW_SYSTEM_CLIENT_WINDOW_MANAGER_H
