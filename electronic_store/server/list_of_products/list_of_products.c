#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "../logger/actions_with_product.h"
#include "list_of_products.h"
#include "../logger/logger.h"
#include "../packet_types.h"
#include "../constants.h"


static pthread_mutex_t list_of_products_mutex_;
static Product_info* head_element = NULL;

//инициализация мьютекса
void init_list_of_products_mutex() {
    pthread_mutex_init(&list_of_products_mutex_, NULL);
}


int is_product_exists_(char* name, int* cost) {
    Product_info* iterator = head_element;

    while (iterator != NULL && strcmp(iterator->name, name) != 0) {
        iterator = iterator->next;
    }

    if (iterator == NULL) {
        return -1;
    }

    *cost = iterator->cost;

    return 1;
}


static Product_info* make_new_list_of_product_item_(char* name, int count, int cost) {
    Product_info* new_item = (Product_info*) malloc(sizeof(Product_info));

    new_item->next = NULL;
    new_item->cost = cost;
    new_item->count = count;
    new_item->name = strdup(name);

    return new_item;
}

//добавление
int list_of_products_add(char* name, int count, int cost) {
    pthread_mutex_lock(&list_of_products_mutex_);

    int product_cost;
    if (is_product_exists_(name, &product_cost) == 1) {
        //если существует товар с таким именем, но другой стоимостью
        if (product_cost != cost) {
            pthread_mutex_unlock(&list_of_products_mutex_);
            return -1;
        }

        Product_info* iterator = head_element;
        while (strcmp(iterator->name, name) != 0) {
            iterator = iterator->next;
        }

        iterator->count += count;
    } else {
        // если такого товара нет в списке
        Product_info* new_product = make_new_list_of_product_item_(name, count, cost);

        if (head_element == NULL) {
            head_element = new_product;
        } else {
            Product_info *iterator = head_element;
            while (iterator -> next != NULL) {
                iterator = iterator->next;
            }
            iterator->next = new_product;
        }

    }

    pthread_mutex_unlock(&list_of_products_mutex_);
    log_product_action("PRODUCT", ADD_PRODUCT, count, name);
    return 1;
}

//покупка (возвращаем кол-во купленного товара)
int list_of_products_remove(char* name, int count) {

    pthread_mutex_lock(&list_of_products_mutex_);

    int number_removed = count;
    Product_info* iterator = head_element;
    Product_info* iterator_prev = NULL;

    while (iterator != NULL && strcmp(iterator->name, name) != 0) {
        iterator_prev = iterator;
        iterator = iterator->next;
    }

    //если не нашли элемент
    if (iterator == NULL) {
        pthread_mutex_unlock(&list_of_products_mutex_);
        return -1;
    }

    // если запрашиваем больше существующего
    if (iterator->count <= count) {
        number_removed = iterator->count;
    }

    iterator->count -= count;

    if (iterator->count <= 0) {
        //если начало списка
        if (iterator_prev == NULL) {
            head_element = head_element->next;
            free(iterator);
        } else {
            iterator_prev->next = iterator->next;
            free(iterator);
        }
    }


    pthread_mutex_unlock(&list_of_products_mutex_);
    log_product_action("PRODUCT", BUY_PRODUCT, count, name);
    return number_removed;
}


//удаление всего списка
void list_of_products_remove_all() {
    pthread_mutex_lock(&list_of_products_mutex_);

    Product_info* iterator = head_element;
    Product_info* iterator_next;

    while (iterator != NULL) {
        iterator_next = iterator->next;
        free(iterator);
        iterator = iterator_next;
    }

    pthread_mutex_unlock(&list_of_products_mutex_);
}


static ulong count_list_of_products_items_length_() {
    ulong result;
    Product_info* iterator;

    for (iterator = head_element,  result = 0; iterator != NULL ; iterator = iterator->next) {
        result += (SIZE_OF_PACKET_PRICE + SIZE_OF_PACKET_COUNT + strlen(iterator->name) + SIZE_OF_ZERO_CHAR);
    }

    return result;
}


// вместо получения всего списка можно сразу реализовать отправку
void list_of_products_send(int client_sockfd) {
    uint32_t packet_length = SIZE_OF_PACKET_LENGTH + SIZE_OF_PACKET_TYPE + count_list_of_products_items_length_();
    uint16_t packet_type = LIST_OF_PRODUCTS_PACKET;
    char zero_char = '\0';

    write(client_sockfd, &packet_length, SIZE_OF_PACKET_LENGTH);
    write(client_sockfd, &packet_type, SIZE_OF_PACKET_TYPE);

    Product_info* iterator = head_element;
    while (iterator != NULL) {
        write(client_sockfd, &(iterator->cost), SIZE_OF_PACKET_PRICE);
        write(client_sockfd, &(iterator->count), SIZE_OF_PACKET_COUNT);
        write(client_sockfd, iterator->name, strlen(iterator->name));
        write(client_sockfd, &zero_char, SIZE_OF_ZERO_CHAR);

        iterator = iterator->next;
    }
}