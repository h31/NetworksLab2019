#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <termios.h>
#include <signal.h>
#include <poll.h>
#include <sys/ioctl.h>

#define size_time 5
#define buff_time 40
//Максимальная длина сообщения
#define buffMessage 512
//Максимальная длина имени пользователя
#define maxNameSize 16
#define port 5001
//Максимальное  разрешенное количество клиентов
#define maxClients 10

//Создаем сокет сервера
int sockfd;

//Структура клиента
typedef struct{
    //У каждого клиента есть сокет, имя
    int socket;
    char *name;

}client;

//Массив клиентов
client *clients[maxClients];
//Инициализируем структуру для опроса входящих соединений
struct pollfd pollClients[maxClients];
//Счетчик клиентов
//Инициализация текущего счетчика клиентов
int countClients = 0;

void printMessage(char* name, char* message){
    /*
    * template for time <00:00> size 5
    * Функция для определения времени отправки сообщения
    */
    time_t timer = time(NULL);
    struct tm* timeStruct = localtime(&timer);
    char stringTime[size_time];
    //bzero(stringTime, size_time);
    int length = strftime(stringTime,buff_time,"%H:%M", timeStruct);
    //Вывод сообщения
    printf("<%s>[%s]: %s\n", stringTime, name, message);

}

void printServerLog(char* message, int state){
    /*
    * template for time <00:00> size 5
    * Функция для определения времени отправки сообщения
    */
    time_t timer = time(NULL);
    struct tm* timeStruct = localtime(&timer);
    char stringTime[size_time];
    //bzero(stringTime, size_time);
    int length = strftime(stringTime,buff_time,"%H:%M", timeStruct);

    //Определяем что отобразить
    switch(state){
        case 0: {
            //Вывод сообщения
            printf("<%s>: %s\n", stringTime, message);
            break;
        }
        case 1: {
            printf("<%s>:Клиент %s инициализирован\n", stringTime, message);
            break;
        }
        case 2: {
            printf("<%s>Сообщение : %s\n", stringTime, message);
            break;
        }
        case 3: {
            printf("<%s>:Имя клиента = %s\n", stringTime, message);
            break;
        }
        case 4: {
            printf("<%s>:Клиент %s отправил сообщение\n", stringTime, message);
        }
    }
}

//Функция закрытие клиента
void closeClient(int socket){
    shutdown(socket,SHUT_RDWR);
    close(socket);
    for (int i = 1; i <= maxClients; i++){
        if((clients[i] !=NULL)  &&(clients[i]->socket==socket)){
            printf("Пользователь %s покинул нас\n", clients[i]->name);
            for (int j = i; j < countClients; j++){
                clients[j] = clients[j+1];
            }
            break;
        }
    }
    for (int i = 1; i <= countClients; i++){
        if (pollClients[i].fd == socket){
            for (int j = i; j < countClients; j++){
                pollClients[j] = pollClients[j+1];
            }
        }
    }
    countClients--;
}

//Функция закрытия сервера
void closeServer(){

    //Отключить всех клиентов
    for (int i =1; i <= countClients; i ++){
        closeClient(pollClients[i].fd);
    }
    shutdown(sockfd,SHUT_RDWR);
    close(sockfd);
    free(clients);
    free(pollClients);
    exit(1);

}

//Отправка сообщений всем клиентам клиентам, кроме себя
void sendMessageClients(char* message, int socket, char* name){

    int n;
    char* sendM = (char *) malloc((strlen(message)+ strlen(name))*sizeof(char));
    strcat(sendM,"[");
    strcat(sendM,name);
    strcat(sendM,"]: ");
    strcat(sendM, message);
    int length = strlen(sendM);

    for (int i = 1; i <=countClients; i++){

        if (pollClients[i].fd != socket){
            n = write(pollClients[i].fd, &length, sizeof(int));
            if (n <= 0){
                closeClient(pollClients[i].fd);
                break;
            }
            n = write(pollClients[i].fd, sendM, length);
            if (n <= 0){
                closeClient(pollClients[i].fd);
                break;
            }
        }

    }

    free(sendM);

}

//Функция для приема сообщений от клиентов
void reciveMessage(int socket, char* bufferMessage,  char * name){
    int n;
    int length = 0;
    //Получа                    closeClient(pollClients[i].fd)ем размер сообщения
    n = read(socket, &length, sizeof(int));

    if (n <= 0) {
        perror("ERROR reading from socket\n");
        closeClient(socket);
    }

    if(length > 0){
        printf("Размер введенного сообщения %d\n", length);
        //Получаем само сообщение
        n = read(socket, bufferMessage, length);
        if (n <= 0) {
            perror("ERROR reading from socket\n");
            closeClient(socket);
        }
        if (n > 0){
            printf("length %d\n", length);
            printMessage(name, bufferMessage);
        }
    }

}

//Обработка сигнала выхода от пользователя
void signalExit(int sig){
    closeServer();
}

int main(int argc, char *argv[]) {

    int newsockfd;
    uint16_t portno;
    unsigned int clilen;
    char bufferMessage[buffMessage];
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;
    int tmp;


    signal(SIGINT, signalExit);

    for(int i = 0; i < maxClients; i++){
        clients[i] = NULL;
    }

    /* Сокет для прослушивания других клиентов */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
    }

    /*Инициализируем сервер*/
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = port;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR,&tmp, sizeof(int))<0){
        perror("ERROR");
        exit(1);
    }

    /*Слушаем клиентов */
    printServerLog("Сервер запущен. Готов слушать",0);
    listen(sockfd, maxClients);
    clilen = sizeof(cli_addr);

    pollClients[countClients].fd = sockfd;
    pollClients[countClients].events = POLLIN;
    countClients++;

    //Работа сервера
    while(1){

        //Опрос клиентов
        tmp = poll(pollClients, (unsigned int) maxClients, 10000);

        for(int i = 0; i < countClients; i++){

            //Если никакое событие не произошло
            if (pollClients[i].revents == 0){
                continue;
            }

            //Если произошло смотрим что не с сокетом который на прием новых клиентов
            if (pollClients[i].fd == sockfd){
                /* Сокет для приёма новых клиентов */
                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                client* newClientInfo = (client*) malloc(sizeof(client));

                pollClients[countClients].fd = newsockfd;
                pollClients[countClients].events = POLLIN;
                printServerLog("Вошел новый клиент", 0);
                //Получаем имя клиента
                int length = 0;
                bzero(bufferMessage, buffMessage);
                //Получаем размер сообщения
                n = read(pollClients[countClients].fd, &length, sizeof(int));
                //Вывод размера введенного сообщения
                printf("Размер введенного сообщения %d\n", length);
                //Выделяем память для клиента
                char *nameClient = (char *) malloc(length);
                bzero(nameClient,length + 1);
                n = read(pollClients[countClients].fd, nameClient, length);

                printServerLog(nameClient,3);

                newClientInfo->socket = newsockfd;
                newClientInfo->name = nameClient;
                //Добавление нового клиента в массив клиентов
                for (int i = 1; i <= countClients; i++){
                    if (clients[i] == NULL){
                        clients[i] = newClientInfo;
                        break;
                    }
                }

                countClients++;

            }
            else{
                bzero(bufferMessage, buffMessage);
                reciveMessage(pollClients[i].fd, (char *) bufferMessage, clients[i]->name);
                sendMessageClients((char *) bufferMessage, pollClients[i].fd, clients[i]->name);
            }
        }
    }
    return 0;
}