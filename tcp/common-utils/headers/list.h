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

#endif //SERVER_LIST_H