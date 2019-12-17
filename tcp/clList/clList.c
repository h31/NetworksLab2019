#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include "clList.h"

client *head = NULL;
pthread_mutex_t mux;            // мьютекс для контроля изменений в списке


void push_el(client *cl) {
    pthread_mutex_lock(&mux);
    if (head == NULL) head = cl;    //если в списке еще нет элементов

    else {                            //если список не  пустой, то добавляем в конец
        client *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = cl;
        current->next->prev = current;
    }
    pthread_mutex_unlock(&mux);
}


int remove_cl(client *cl) {
    int res = 0;
    pthread_mutex_lock(&mux);
    if (head == NULL) {
        perror("ERROR on removing: list is empty");
        exit(1);
    }
    client *current = head;

    while (current == cl) {
        current = current->next;
        if (current->next == NULL) {     //прошли до конца списка
            perror("Error on removing. No such client in the list.");
            exit(1);
        }
    }
    //нашли клиента в списке
    current->state = false; // меняем статус: клиент отключен
    if (current->next == NULL) {         //удаляемый -последний элемент в списке
        current->prev->next = NULL;

    } else if (current->prev == NULL) {    //удаляемый -первый в списке
        head = current->next;
    } else {
        current->prev->next = current->next;
        current->next->prev = current->prev;
    }
    close(current->socket); //закрываем сокет клиента
    free(current);
    pthread_mutex_unlock(&mux);
    return res;
}


void broadcast_to_list(packet *p) {
    pthread_mutex_lock(&mux);
    if (head == NULL) {
        perror("ERROR on sending: list of clients is empty");
        exit(1);
    }
    client *current = head;
    while (current->next != NULL) {
        write(current->socket, &(p->size), LEN_MESSAGE); // Передаем размер сообщения
        write(current->socket, p->text, p->size); //само сообщение
        current = current->next;

    }
    pthread_mutex_unlock(&mux);
    free(p->text);
    free(p);
}

//Snader Effective TCP programming listing 2.12
int readn(int fd, char *bp, size_t len) {
    size_t cnt;
    ssize_t rc;
    cnt = len;
    while (cnt > 0) {
        rc = recv(fd, bp, cnt, 0);
        if (rc < 0) {                /* Ошибка чтения?*/
            if (errno == EINTR)    /* Вызов прерван*/
                continue;            /* Повторить чтение*/
            return -1;                /*Вернуть код ошибки */
        }
        if (rc == 0)                /* Конец файла*/
            return (int) (len - cnt);        /* Вернуть неполный счетчик */
        bp += rc;
        cnt -= rc;
    }
    return (int) len;
}

void check_line(char *buf, int len, char check, char replace) {
    for (int i = 0; i < len; ++i) {
        if (buf[i] == check) buf[i] = replace;
    }
}

client *get_header() { return head; }

void set_header(client *h) {
    head = h;
    return;
}

pthread_mutex_t get_mutex() { return mux; }

void set_mutex(pthread_mutex_t m) {
    mux = m;
    return;
}
