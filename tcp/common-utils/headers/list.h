#ifndef SERVER_LIST_H
#define SERVER_LIST_H

#include "common.h"
#include "errors.h"

typedef struct List {
    int size;
    struct Node *node;
} List;

typedef struct Node {
    Client *client;
    struct Node *next;
} Node;

void push(List *list, Client *client);

void delete(List *list, Client *client);

Client *get_by_id(List *list, int id);

void foreach(void (*f)(int, char *), char *info, List *list);

#endif //SERVER_LIST_H