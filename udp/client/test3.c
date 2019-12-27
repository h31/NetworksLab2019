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
#define port 5004
//Длина записи
#define lengthMess 256
//Максимальная длина сообщения
#define buffMessage 256
//Максимальное количество записей вакансий
#define maxOffers 1024
//Максимальное число критериев 
#define maxCond 5
//Максимальное число специальностей
#define maxProff 1024
//Массив критериев для поиска
char **findConditon;
//Массив специальностей
char **proffesion;
//Стартовый idProffesion
int nextId = 1;
//Определение мьютекса
pthread_mutex_t mutex;
//Создаем сокет прослушивающий входящих клиентов
int sockfd;

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
int massId[maxOffers];
int countOfDel = 0;
//Структура описывающая вакансию
typedef struct{
    int id; //id записи
    char *proff; // Специальность
    char *positon; // Должность
    int sall; //ЗП
    int age; // Возраст
    char *company; // Компания

} offers;
//Массив вакансий
offers* massOffers[maxOffers];
//Счетчик специальностей
int countProff = 0;
//Счетчик вакансий
int countOffer = 0;

//Название специальности
char *tempName;
//Массив совпадений
int masIndexCond[maxOffers];
//Счетчик элементов этого массива
int countCond = 0;

void initCond(){
    findConditon = malloc(maxCond*sizeof(void *));
    for (int i = 0; i < maxCond; i++){
        findConditon[i] = (char *) malloc (lengthMess);
    }
}

void initProffesion(){
    proffesion = malloc(maxProff*sizeof(void *));
    for (int i = 0; i < maxProff; i++){
        proffesion[i] = (char *) malloc (lengthMess);
    }
}

void addProff(char* prof){
    if (countProff < maxProff){
        proffesion[countProff] = strdup(prof);
        printf("Специальность: <%s> добавлена\n", proffesion[countProff]);
        countProff++;
    }
}

//Вывод новости на экран
void printVac(){
    for(int i = countOffer - 1; i < countOffer; i ++){
        printf("[ID]: <%d>\n", massOffers[i]->id);
        printf("[Профессия]: <%s>\n", massOffers[i]->proff);
        printf("[Должность]: <%s>\n", massOffers[i]->positon);
        printf("[Зарплата]: <%d>\n", massOffers[i]->sall); 
        printf("[Возраст]: <%d>\n", massOffers[i]->age); 
        printf("[Компания]: <%s>\n", massOffers[i]->company); 
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
    //Чистим массив вакансий
    for(int i = 0; i < countOffer; i++){
        massOffers[i] = NULL;
    }
    //Чистим массив условий
    for(int i = 0; i < countOffer; i++){
        free(findConditon[i]);
    }
    //Чистим массив специальностей
    for(int i = 0; i < countProff; i++){
        free(proffesion[i]);
    }
    free(findConditon);
    free(proffesion);
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
            printf("Запрос на вывод вакансий\n");
            break;
        }
        case 2: {
            printf("Запрос на вывод профессий\n");
            break;
        }
        case 3: {
            printf("Запрос на удаление \n");
            tempName = partRead(socket, tempName);
            printf("Удалить специальность <%s>\n", tempName);
            break;
        }
        case 4: {
            //Обработка запроса на поиск
            printf("Запрос на поиск по критериям\n", flag);
            //Получаем специальность
            buffer = partRead(socket, buffer);
            findConditon[0] = strdup(buffer);
            //Получаем компанию
            buffer = partRead(socket, buffer);
            findConditon[1]= strdup(buffer);
            //Получаем зарплату
            buffer = partRead(socket, buffer);
            findConditon[2] = strdup(buffer);
            //Получаем должность
            buffer = partRead(socket, buffer);
            findConditon[3]= strdup(buffer);
            //Получаем возраст
            buffer = partRead(socket, buffer);
            findConditon[4] = strdup(buffer);
            printf("Осуществить поиск по следующим критериям\n");
            printf("----------------------------------------\n");

            printf("Специальность: <%s> Длина строки <%d>\n", findConditon[0],strlen(findConditon[0]));
            printf("Компания: <%s> Длина строки <%d>\n", findConditon[1],strlen(findConditon[1]));
            printf("Зарплата: <%s> Длина строки <%d>\n", findConditon[2],strlen(findConditon[2]));
            printf("Должность: <%s> Длина строки <%d>\n", findConditon[3],strlen(findConditon[3]));
            printf("Возраст: <%s> Длина строки <%d>\n", findConditon[4],strlen(findConditon[4]));

            printf("-----------------------------------------\n");
            break;
            
        }
        case 5: {
            //добавление специальности
            printf("Запрос на добавление специальности\n", flag);
            printf("Начинаем получать \n");
            // news* nNews = (news *) malloc (sizeof(news));
            char *buffer; 
            //Получаем специальность
            buffer = partRead(socket, buffer);
            addProff(buffer);
            free(buffer);
            break;
    
        }
        case 6: {
            //добавление вакансии
            printf("Запрос на добавление вакансии\n", flag);
            printf("Начинаем получать \n");
            offers* newOffer = (offers *) malloc (sizeof(offers));
            //Получаем специальность
            buffer = partRead(socket, buffer);
            newOffer->proff = strdup(buffer);
            //Получаем должность
            buffer = partRead(socket, buffer);
            newOffer->positon = strdup(buffer);
            int salary = 0;
            n = read(socket, &salary, sizeof(int));
            if (n <= 0) {
                perror("ERROR reading from socket\n");
                closeClient(socket);
            }
            newOffer->sall = salary;
            //Получаем возраст
            int age = 0;
            n = read(socket, &age, sizeof(int));
            if (n <= 0) {
                perror("ERROR reading from socket\n");
                closeClient(socket);
            }
            newOffer->age = age;
            //Получаем компанию
            buffer = partRead(socket, buffer);
            newOffer->company = strdup(buffer);
            newOffer->id = nextId;
            printf("id = %d\n", newOffer->id);
            massOffers[countOffer] = newOffer;
            countOffer++;
            nextId++;
            //Проверка на то что профессия есть
            for (int i = 0; i < countProff; i++){
                
                if (!strcmp(newOffer->proff, proffesion[i])){
                    break;
                }
                if (i == countProff-1){
                    proffesion[countProff] = strdup(newOffer->proff);
                    countProff++;
                    break;
                }
            }
            if (countProff == 0){
                proffesion[countProff] = strdup(newOffer->proff);
                countProff++;
            }
            //Прочитанная вакансия
            printVac();
            free(buffer);
            break;
        }
    }
    return flag;
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

            //Отправили количество вакансий
            n = write(socket, &countOffer, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }
            for(int i =0; i < countOffer; i++){
                //Отправляем название специальности
                writePart(socket, massOffers[i]->proff);
                //Отправляем название должности
                writePart(socket, massOffers[i]->positon);
                //Отправляем зарплату
                n = write(socket, &massOffers[i]->sall, sizeof(int));
                if (n < 0) {
                    perror("ERROR writing to socket");
                    closeClient(socket);
                }
                //Отправляем возраст
                n = write(socket, &massOffers[i]->age, sizeof(int));
                if (n < 0) {
                    perror("ERROR writing to socket");
                    closeClient(socket);
                }
                //Отправляем название компании
                writePart(socket, massOffers[i]->company);
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

            //Отправили количество профессий
            n = write(socket, &countProff, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }
           
            for (int i = 0; i < countProff; i++){
                writePart(socket, proffesion[i]);             
            }
            break;
        }
        case 3: {
            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }
            char *tempstr = "Proffesion deleted";
            writePart(socket, tempstr);
            break;
        }
        case 4: {
            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }

            //Отправили количество вакансий
            n = write(socket, &countCond, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }
            for(int i =0; i < countCond; i++){
                //Отправляем название специальности
                writePart(socket, massOffers[masIndexCond[i]- 1]->proff);
                //Отправляем название должности
                writePart(socket, massOffers[masIndexCond[i] - 1]->positon);
                //Отправляем зарплату
                n = write(socket, &massOffers[masIndexCond[i]- 1]->sall, sizeof(int));
                if (n < 0) {
                    perror("ERROR writing to socket");
                    closeClient(socket);
                }
                //Отправляем возраст
                n = write(socket, &massOffers[masIndexCond[i]-1]->age, sizeof(int));
                if (n < 0) {
                    perror("ERROR writing to socket");
                    closeClient(socket);
                }
                //Отправляем название компании
                writePart(socket, massOffers[masIndexCond[i]-1]->company);
            }
            countCond = 0;
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
            char *tempstr = "Proffesion accepted";
            writePart(socket, tempstr);
            break;
        }
        case 6: {
            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n <= 0) {
                perror("ERROR writing to socket");
                closeClient(socket);
            }          
            char *tempstr = "Vacansion accepted";
            writePart(socket, tempstr);
            break;
        }
    }

}
void findIdOffer(){
    
    for (int i = 0; i < countOffer; i++){
        if(!strcmp(tempName, massOffers[i]->proff)){
            massId[countOfDel] = i;
            countOfDel++;
        }
    }

}
//подумать
void deletedOffer(){
    offers* newMassOffers[maxOffers];
    int tempCount = 0;
    for (int i = 0; i < countOfDel; i++){
        massOffers[massId[i]] = NULL;
        massId[i] = 0;
        countOffer--;
    }
    for(int i = 0; i < maxOffers; i++){
        if(massOffers[i]!= NULL){
            newMassOffers[tempCount]=massOffers[i];
            tempCount++;
        }
    }
    for(int i =0; i < maxOffers; i++){
        massOffers[i] = NULL;
    }
    for (int i =0; i < countOffer;i++){
        massOffers[i] = newMassOffers[i];
    }

}
//Функция реализующая удаление
void deletProff(){
    for (int i = 0; i < countProff; i++){
        if(!strcmp(tempName, proffesion[i])){
            free(proffesion[i]);
            for(int j = i; j < countProff; j++){
                proffesion[j] = strdup(proffesion[j+1]);
            }
            break;
        }
    }
    
    findIdOffer();
    deletedOffer();
    countProff--;
}

int comp(char *str1, char *str2){
    if (strlen(str1) == 0)
        return 0;
    if (!strcmp(str1,str2)){
        return 0;
    }
    return 1;
}
int cmpNum(int a, int b){
    if (a == 0){
        return 0;
    }
    if (a == b){
        return 0;
    }
    return 1;
}

//функция осуществялющая фильтрацию
void findCondOffer(){
    int salary = (int) atoi(findConditon[2]);
    int age = (int) atoi(findConditon[4]);
    
    for(int i = 0; i < maxOffers; i++){
        masIndexCond[i] = 0;
    }
    
    for (int i =0; i < countOffer; i++){
        if (!comp(findConditon[0], massOffers[i]->proff) && !comp(findConditon[1], massOffers[i]->company) && !cmpNum(salary, massOffers[i]->sall) && !comp(findConditon[3], massOffers[i]->positon) && !cmpNum(age, massOffers[i]->age)){
            masIndexCond[countCond] = massOffers[i]->id;
            countCond++;
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
                printf("Отправляем вакансии клиенту\n");
                sendResponseToClient(socket,flag);          
                break;
            }
            case 2: {
                printf("Отправляем специальности клиенту\n");
                sendResponseToClient(socket, flag);
                break;
            }
            case 3: {
                printf("Запрос на удаление получен\n");
                deletProff();
                sendResponseToClient(socket,flag);
                free(tempName);
                break;
            }
            case 4: {
                //Запрос на поиск
                printf("Пришел запрос на поиск по параметрам: \n");
                
                findCondOffer();
                for (int i = 0; i < countCond; i++){
                    if (masIndexCond[i]>0){
                        printf("index  %i\n", masIndexCond[i]);
                    }                    
                }
                sendResponseToClient(socket,flag); //--Данные записаны
                printf("Ответ клиенту отправлен\n");

                break;
            }
            case 5: {
                //Добавление специальности
                printf("Запрос на добавление специальности\n");
                sendResponseToClient(socket,flag);
                break;
            }
            case 6: {
                sendResponseToClient(socket,flag); //--Данные записаны
                printf("Ответ клиенту отправлен, вакансия создана\n");
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

    //char beffTheme[sizeTheme];

    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;
    
    signal(SIGINT, signalExit);
    //Инициализация мьютекса
    pthread_mutex_init(&mutex,NULL);
    
    //Идентификатор потока
    pthread_t clientTid;

    //Инициализируем массивы
    initCond();
    initProffesion();
    for(int i =0; i < maxOffers; i++){
        massId[i] = 0;
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