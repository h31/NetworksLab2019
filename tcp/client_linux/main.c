#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

int sendContent(int destination, char* content);
void* handleMessages(void* args);
int readMessage(int fromSock, char* buffer);
int readN(int socket, char* buf, int length);

int sockfd;

int main(int argc, char *argv[]) {

    printf("Client started with PID = %d\n", getpid());

    //Переменная под сокет
    int n;
    //Номер порта
    uint16_t portno;
    //Адрес сервера
    struct sockaddr_in serv_addr;
    //Информация о сервере
    struct hostent *server;
    //Имя пользователя
    char* nickname;

    //Буфер под сообщения
    char buffer[256];

    //Проверка на количество аргументов
    if (argc < 4) {
        fprintf(stderr, "usage %s hostname port nickname\n", argv[0]);
        exit(0);
    }

    //Преобразуем порт из строки в int
    portno = (uint16_t) atoi(argv[2]);

    //Открываем сокет
    printf("Открываю сокет\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //Проверяем корректно ли открылся сокет
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    //Получаем информацию о хосте с помощью адреса
    printf("Получаю информацию о хосте\n");
    server = gethostbyname(argv[1]);

    //Проверяем удалось ли получить информацию о хосте
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    //Читаем имя пользователя из аргумента
    printf("Считываю никнейм\n");
    nickname = argv[3];

    //Проверяем не пустое ли имя пользователя
    if (strcmp(nickname, "") == 0)
    {
        fprintf(stderr, "ERROR, nickname is empty\n");
        exit(0);
    }

    //На всякий случай обнуляем адрес сервера
    bzero((char *) &serv_addr, sizeof(serv_addr));
    //Задаём семейство адресов сервера
    serv_addr.sin_family = AF_INET;
    //Копируем адрес в переменную сокета
    printf("Копирую адрес\n");
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    //Определяем сетевой порядок байт для порта
    serv_addr.sin_port = htons(portno);

    printf("Подключаюсь к серверу\n");
    //Подключаемся к серверу
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    sendContent(sockfd, nickname);

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    pthread_t tid;
    if (pthread_create(&tid, NULL, handleMessages, NULL) != 0) {
        printf("thread has not created");
        exit(1);
    }

    for (;;)
    {
        printf("Введите сообщение: ");
        bzero(buffer, sizeof(buffer));
        fgets(buffer, sizeof(buffer) - 1, stdin);
        sendContent(sockfd, buffer);
    }
    
    return 0;

}

void* handleMessages(void* args){

    char* readBuffer[300];

    for (;;){
        if (readMessage(sockfd, readBuffer) <= 0){
            break;
        }
        printf(readBuffer);
        fflush(stdout);
    }
}

int readN(int socket, char* buf, int length){
    int result = 0;
    int readedBytes = 0;
    int sizeMsg = length;
    while(sizeMsg > 0){
        readedBytes = read(socket, buf + result, sizeMsg);
        if (readedBytes <= 0){
            return -1;
        }
        result += readedBytes;
        sizeMsg -= readedBytes;
    }
    return result;
}

int readMessage(int fromSock, char* buffer){
    bzero(buffer, sizeof(buffer));
    int tmp;
    int size;
    tmp = readN(fromSock, &size, sizeof(int));

    if (tmp < 0) {
        perror("ERROR reading from socket");
        return tmp;
    }

    tmp = readN(fromSock, buffer, size);

    if (tmp < 0) {
        perror("ERROR reading from socket");
        return tmp;
    }

    return tmp;
}

int sendContent(int destination, char* content){
    int check;
    printf("Отправляю длину сообщения\n");
    int size = strlen(content);
    check = write(destination, &size, sizeof(int));
    if (check <= 0)
    {
        printf("Ошибка передачи\n");
    }
    printf("Отправляю сообщение\n");
    check = write(destination, content, size);
    if (check <= 0)
    {
        printf("Ошибка передачи\n");
    }
}
