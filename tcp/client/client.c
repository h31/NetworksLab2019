#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>

//Номер порта и hostname по умолчанию
#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"
#define DEF_NAME "Unknown"

//Имя пользователя и размер сообщения в байтах
#define NAME_LENGTH 16 
#define MESS_SIZE 4 
#define MESS_LENGTH 512

char buf[MESS_LENGTH];

void replace(char *buff, size_t size, char old, char new);
int readN(int sock, char *buf, int len, int flags);
void *reading(void *args);
void readFix(int sock);

int main(int argc, char** argv) {
    int sock, num;
    uint16_t port;
    char userName[NAME_LENGTH];
    struct hostent *serv;
    struct sockaddr_in peer;
    if (argc < 4) {
        printf("Неполный вызов. Необходимо: ./client.o hostname port name. Будут использованы параметры по умолчанию:\n");
	printf("Hostname: %s, port: %d, name: %s\n", DEF_IP, DEF_PORT, DEF_NAME);
        port = (uint16_t) DEF_PORT;
	serv = gethostbyname(DEF_IP);
	strcpy(userName, DEF_NAME);
    } else { 
    	port = (uint16_t) atoi(argv[2]);
    	serv = gethostbyname(argv[1]);
    	strcpy(userName, argv[3]);
    }

    //Проверяю, есть ли такой хост
    if (serv == NULL) {
        printf("Ошибка! Несуществующий hostname!\n");
        exit(0);
    }
    //Создаю сокет и инициализирую параметры
    sock = socket(AF_INET, SOCK_STREAM, 0);
    //Проверяю, создан ли сокет
    if (sock < 0) {
        printf("Ошибка открытия сокета!\n");
        exit(1);
    }

    bzero((char *) &peer, sizeof(peer));
    peer.sin_family = AF_INET;
    bcopy(serv->h_addr, (char *) &peer.sin_addr.s_addr, (size_t) serv->h_length);
    peer.sin_port = htons(port);

    //Присоединяемся к серверу
    int res = connect(sock, (struct sockaddr*)&peer, sizeof(peer));
    if (res < 0) {
        perror("Не удалось подключиться к серверу!\n");
        exit(1); 
    } else {
        printf("Успешное подключение к серверу.\n");
    }

    //Отправляем сообщение серверу
    num = write(sock, userName, NAME_LENGTH);
    if (num <= 0) {
        perror("Ошибка записи в сокет!\n");
        exit(1);
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, reading, &sock);

    for (;;) {
        //Читаем ответы сервера
        readFix(sock);
    }
    return 0;
}

//Функция для замены символов переноса строки, который после \n
void replace(char *buff, size_t size, char old, char new) {
    for (unsigned i = 0; i < size; ++i) {
        if (buff[i] == old) buff[i] = new;
    }
}

void *reading(void *args) {
    ssize_t num; //Сколько записали
    int sock = *(int *) args;
    for (;;) {
	//Обнуляем буфер
        bzero(buf, MESS_LENGTH);
	//Считываем в буфер
        fgets(buf, MESS_LENGTH, stdin);
        replace(buf, MESS_LENGTH, '\n', '\0');
        //Отправляем серверу
        size_t len = strlen(buf) + 1;
	//Сначала посылаем размер сообщения
        num = write(sock, &len, MESS_SIZE); 
        if (num <= 0) {
            perror("Ошибка записи в сокет! Размер сообщения!\n");
            exit(-1);
        }
	//А затем посылаем само сообщение
        num = write(sock, &buf, len); // Передаем само сообщение
        if (num <= 0) {
            perror("Ошибка записи в сокет! Само сообщение!\n");
            exit(-1);
        }
    }
}

void readFix(int sock) {
    ssize_t num; //Сколько считали
    size_t mesSize = 0;
    //Читаем размер сообщения
    num = readN(sock, (char *)&mesSize, MESS_SIZE, 0);
    //Проверяем, жив ли сервер
    if (num <= 0) {
	//Если нет - сообщаем, что отключился
        perror("Cервер недоступен :c \n");
        exit(0);
    } else {
        //Если жив, то читаем само сообщение
        char *mess = malloc(sizeof(char) *mesSize);
        //Читаем то кол-во сиволов, которое указано в заголовке сообщения
        readN(sock, mess, mesSize, 0);
        printf("\r%s\n", mess);
	//Освобождаем память
        free(mess);
    }
}

//Функция для чтения ровно N байт
//Данный вариант найден здесь: http://www.informit.com/articles/article.aspx?p=169505&seqNum=9
int readN(int sock, char *buf, int len, int flags) {
    int curLen; //Текущая длина сообщения
    int recvMy;
    //В первый раз длина та, какую получила функция
    curLen = len; 
    while (curLen > 0) {
        recvMy = recv(sock, buf, curLen, flags);
        if (recvMy < 0) {
	    //Если ошибка прерывания, то читаем снова
	    //Код ошибки прерывания (EINTR) изучен здесь: https://ru.wikipedia.org/wiki/Errno.h
            if (errno == EINTR)    
                continue;
	    //Если нет - ошибка          
            return -1;              
        }
	//Если дочитали до конца
        if (recvMy == 0)                
            return (len - curLen); 
        buf = buf + recvMy;
        curLen = curLen - recvMy;
    }
    //Вернем столько, сколько прочитали
    return len;
}
