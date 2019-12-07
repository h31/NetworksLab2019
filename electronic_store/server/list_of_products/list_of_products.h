
#ifndef ELECTRONIC_STORE_LIST_OF_PRODUCTS_H
#define ELECTRONIC_STORE_LIST_OF_PRODUCTS_H

typedef struct product_info {
    char* name;
    int cost;
    int count;
    struct product_info* next;
} Product_info;

int list_of_products_add(char* name, int count, int cost);

int list_of_products_remove(char* name, int count);

void list_of_products_send(int client_sockfd);

void init_list_of_products_mutex(void);

void list_of_products_remove_all(void);

#endif //ELECTRONIC_STORE_LIST_OF_PRODUCTS_H
