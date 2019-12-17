#ifndef    CL_LIST_H
#define    CL_LIST_H
#define NAME_L    20        //макс длина имени пользователя
#define BUF_SIZE 512    //размер буфера
#define LEN_MESSAGE 4    //4 байта

typedef struct client {
    int socket;
    char *name;                // имя клиента
    struct sockaddr_in *addr;    //адрес клиента
    bool state;                    //true- подключен, false - отключен
    struct client *next;        //ссылка на следующего клиента
    struct client *prev;        //ссылка на предыдущего клиента
} client;                     //определили тип клиент

typedef struct packet {
    u_int32_t size; // размер сообщения
    char *name; // кто написал
    char *text; //что написал
} packet;


void print_list();

void push__first(client *cl);

void push_el(client *cl);

int remove_cl(client *cl);

void broadcast_to_list(packet *p);

int readn(int fd, char *bp, size_t len);

void check_line(char *buf, int len, char check, char replace);

client *get_header();

void set_header(client *h);

pthread_mutex_t get_mutex();

void set_mutex(pthread_mutex_t m);

#endif
