#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <termios.h>
#include <signal.h>

#define size_time 5
#define buff_time 40
//Максимальное  разрешенное количество клиентов
#define maxClients 10
//Максимальная длина новости
#define buffNew 512
//Максимальная длина темы
#define sizeTheme 128
#define port 5000
#define maxNews 1024

//Начальное значение количества тем
int countThemes = 256;
//Счетчки новостей 
int countNews = 0;

//Массив тем
char **themes;

//Инициализация массива тем
void initMassiveTheme(){
    themes = malloc (countThemes * sizeof(void*));
    for (int i = 0; i < countThemes; i++){
        themes[i] = (char *) malloc (sizeTheme*sizeof(char));
    }
}

char *tempThemes;
char *tempName;
//Определение мьютекса
pthread_mutex_t mutex;
//Создаем сокет прослушивающий входящих клиентов
int sockfd;

//Структура описывающая новость
typedef struct{
    int id; //id 
    char *name; // Имя новости
    char *textNews; // текст новости
    char *themes; // К какой теме принадлежит новость 

} news;

//Массив новостей
news *massNews[maxNews];

//Структура клиента
typedef struct{
    //У каждого клиента есть сокет
    int socket;

}client;

//Массив клиентов
client *clients[maxClients];

//Счетчик клиентов 
//Инициализация текущего счетчика клиентов
int countClients = 0;
//Инициализатор счетчика тем
int countT = 0;

void addThemes(char* theme){

    if (countT != countThemes){
        themes[countT] = strdup(theme);
        printf("Полученная тема от клиента: %s\n", themes[countT]);
        countT++;
    }

}

//Вывод новости на экран
void printNews(){
    for(int i = countNews-1; i < countNews; i ++){
        printf("Тема: %s\n", massNews[i]->themes);
        printf("Название: %s\n", massNews[i]->name);
        printf("Текст: %s\n", massNews[i]->textNews); 
    }
    
}

//Функция закрытие клиента
void closeClient(int socket){
    
    pthread_mutex_lock(&mutex);
    close(socket);
    for (int i = 0; i < maxClients; i++){
        if((clients[i] !=NULL)  &&(clients[i]->socket==socket)){
            printf("Клиент с скоетом %d покинул нас\n", clients[i]->socket);
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);

}

//Функция закрытия сервера 
void closeServer(){
    
    //Отключить всех клиентов
    for (int i =0; i < countClients; i ++){
        closeClient(clients[i]->socket); 
    }
    //Удаляем все записи с темами
    for (int i = 0; i < countT; i++){
        free(themes[i]);
    }
    close(sockfd);
    exit(1);

}

//Чтение части пакета
char* partRead(int socket, char* buffer){
    int length = 0;
    int n;
    n = read(socket, &length, sizeof(int));
    buffer = (char*) malloc ((length+1)*sizeof(char));
    bzero(buffer,length+1);
    if (n < 0) {
        perror("ERROR writing to socket");
        closeClient(socket);
    }
    n = read(socket, buffer, length);
    if (n < 0) {
        perror("ERROR writing to socket");
        closeClient(socket);
    }
    return buffer;
}

int reciveRequestFromClient(int socket,char *buffer){
    
    int flag = 0;
    int n;
    //Получаем значение флага
    n = read(socket, &flag, sizeof(int));
    if (n <= 0 || flag == 0) {
        perror("ERROR reading from socket\n");
        closeClient(socket);
    }

    switch(flag){
        case 1: {
            printf("Запрос на вывод тем\n");
            break;
        }
        case 2: {
            printf("Запрос на вывод новостей по теме\n");
            tempThemes = partRead(socket, tempThemes);
            printf("Отправить новости по теме %s\n", tempThemes);
            break;
        }
        case 3: {
            printf("Запрос на вывод новости\n");
            tempName = partRead(socket, tempName);
            printf("Отправить новость %s\n", tempName);
            break;
        }
        case 4: {
            //добавление тем
            printf("Запрос на добавление тем\n", flag);
            printf("Начинаем получать \n");
            
            int tempCount = 0;
            n = read(socket, &tempCount, sizeof(int));
            printf("Количество введенных тем = %d\n",tempCount);
            if (n <= 0) {
                perror("ERROR reading from socket\n");
                closeClient(socket);
            }
            for (int i = 0; i < tempCount; i++){
                //теперь саму тему
                buffer = partRead(socket,buffer);
                addThemes(buffer);
                free(buffer);
            }
            break;
        }
        case 5: {
            //добавление новостей
            printf("Запрос на добавление новости\n", flag);
            printf("Начинаем получать \n");
            news* nNews = (news *) malloc (sizeof(news));
            char *buffer; 
            //Получаем тему
            buffer = partRead(socket, buffer);
            nNews->themes = strdup(buffer);

            //Получаем заголовок
            buffer = partRead(socket, buffer);
            nNews->name = strdup(buffer);
            //Получаем текст
            buffer = partRead(socket, buffer);
            nNews->textNews = strdup(buffer);

            massNews[countNews] = nNews;
            countNews++;
            //Проверка на то что тема есть, если нет добавляет в массив тем
            for (int i =0; i < countT; i++){
                if (!strcmp(nNews->themes, themes[i])){
                    break;
                }
                if (i == countT-1){
                    themes[i+1] = strdup(nNews->themes);
                    countT++;
                }
            }
            //Прочитанная новость
            printNews();
            free(buffer);
            break;
        }
    }
    return flag;
}

//Вывод количества новостей по теме
int findNews(){
    int res = 0;
    for (int i = 0; i < countNews; i++){
        if (!strcmp(massNews[i]->themes, tempThemes)){
            res++;
        }
    }
    return res;
}

//Отправка части пакета
void writePart(int socket, char *message){
    int n;
    int length = strlen(message);
    n = write(socket, &length, sizeof(int));
    if (n < 0) {
        perror("ERROR writing to socket");
        closeClient(socket);
    }
    //теперь саму тему
    n = write(socket, message, length);
    if (n < 0) {
        perror("ERROR writing to socket");
        closeClient(socket);
    }
}

news* findCurNews(){
    for (int i = 0; i < countNews; i++){
        if (!strcmp(massNews[i]->name, tempName)){
            return massNews[i];
        }
    }
    return NULL;
}

void sendResponseToClient(int socket, int flag){

    switch(flag){
        case 1: {
            int n;
             //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }
            //Отправили количество тем
            n = write(socket, &countT, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }
            for (int i = 0; i < countT; i++){
                writePart(socket, themes[i]);     
            }
            break;
        }
        case 2: {
            int n;
             //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }

            int countNew = findNews();
            printf("Количество новостей по данной теме %d\n", countNew);

            //Отправили количество тем
            n = write(socket, &countNew, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }
           
            for (int i = 0; i < countNews; i++){
                printf("Новость %s\n",massNews[i]->name);
                if (!strcmp(massNews[i]->themes, tempThemes)){
                    writePart(socket, massNews[i]->name);
                }               
            }
            break;
        }
        case 3: {
            int n;
            
            news* new = findCurNews();

            if (new != NULL){
                //Отправили флаг
                n = write(socket, &flag, sizeof(int));
                if (n < 0) {
                    perror("ERROR writing to socket");
                    closeClient(socket);
                }
                //Сначала отправляем тему
                writePart(socket,new->themes);
                //Отправка заголовка 
                writePart(socket,new->name);
                //Отправка текста
                printf("text = %s\n",new->textNews);
                writePart(socket,new->textNews);           
                new = NULL;
                printf("Отправлено\n");
            }
            else{
                int err = 6;
                //отправляем сообщение об ошибке
                n = write(socket, &err, sizeof(int));
                if (n < 0) {
                    perror("ERROR writing to socket");
                    closeClient(socket);
                }
                char *tempstr = "News is missing";
                writePart(socket, tempstr);
            }
            break;
        }
        case 4: {

            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n <= 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }          
            char *tempstr = "Data accept";
            writePart(socket, tempstr);
            break;

        }
        case 5: {
            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n <= 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }          
            char *tempstr = "Data accept";
            writePart(socket, tempstr);
            break;
        }
    }

}



//Обработчик потока клиента
void* clientWorks(void* clientI){
    
    //Инициализация
    client *clientInfo = *(client**) clientI;
    char *buffer;

    int socket = clientInfo->socket;
    int flag = 0;
    int length = 0;


    //Слушаем запросы от клиента и даем ему ответы
    while(1){
        flag = reciveRequestFromClient(socket,buffer);
        switch(flag){
            case 1: {
                printf("Отправляем темы клиенту\n");
                sendResponseToClient(socket,flag);          
                break;
            }
            case 2: {
                printf("Отправляем новости клиенту\n");
                sendResponseToClient(socket, flag);
                free(tempThemes);
                break;
            }
            case 3: {
                printf("Отправляем новость клиенту\n");
                sendResponseToClient(socket,flag);
                free(tempName);
                break;
            }
            case 4: {
                sendResponseToClient(socket,flag); //--Данные записаны
                printf("Ответ клиенту отправлен, темы получены и записаны\n");
                break;
            }
            case 5: {
                break;
            }
            default: {
                sendResponseToClient(socket,flag); //--Данные записаны
                printf("Ответ клиенту отправлен, новость получена и записана\n");
                break;
            }
        
        }   
        
        flag = 0;
    }
}   

//Инициализация массива клиентов
void initMas(){
    for(int i = 0; i < maxClients; i++){
        clients[i] = NULL;
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

    char beffTheme[sizeTheme];

    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;
    
    signal(SIGINT, signalExit);
    //Инициализация мьютекса
    pthread_mutex_init(&mutex,NULL);
   
    //Инициализация массива клиентов
    initMas();

    initMassiveTheme();
    
    //Идентификатор потока
    pthread_t clientTid;

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

    /*Слушаем клиентов */
    printf("Сервер запущен. Готов слушать\n");
    
    listen(sockfd, maxClients);
    clilen = sizeof(cli_addr);

    //Работа сервера
    while (1){
        
        /* Сокет для приёма новых клиентов */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        client* newClientInfo = (client*) malloc(sizeof(client));
       
        if (newsockfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }
        else{
            pthread_mutex_lock(&mutex);
                    
            newClientInfo->socket = newsockfd;

            //Добавление нового клиента в массив клиентов
            for (int i = 0; i <= countClients; i++){
                if (clients[i] == NULL){
                    clients[i] = newClientInfo;
                    break;
                }
            }
          
            pthread_mutex_unlock(&mutex);
              
            //Создаем поток для клиента
            pthread_create(&clientTid, NULL, clientWorks, &newClientInfo);

            countClients++;
        }
 
    }
    
    return 0;
}