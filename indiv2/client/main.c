#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>

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

//Структура описывающая вакансию
typedef struct{
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

//Тема по которой ищем новость
char *tempThemes;
//Имя по которой полкчаем новость
char *tempName;


//Функция закрытия клиента
void stopClient(int socket){
    shutdown(socket,SHUT_RDWR);
    close(socket);
    exit(1);
}

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

//Добавление темы
void addThemes(char* theme){
}

//Добавление новости
void addNews(char* name){
}

void addProff(char* prof){
    if (countProff < maxProff){
        proffesion[countProff] = strdup(prof);
        countProff++;
    }
}
//Чтение части пакета
char* partRead(int socket, char* buffer){
    int length = 0;
    int n;
    n = read(socket, &length, sizeof(int));
    buffer = (char*) malloc ((length+1)*sizeof(char));
    bzero(buffer,(length+1));
    if (n < 0) {
        perror("ERROR writing to socket");
        stopClient(socket);
    }
    n = read(socket, buffer, length);
    if (n < 0) {
        perror("ERROR writing to socket");
        stopClient(socket);
    }
    return buffer;
}

//Отправка части пакета
void writePart(int socket, char *message){
    int n;
    int length = strlen(message);
    n = write(socket, &length, sizeof(int));
    if (n < 0) {
        perror("ERROR writing to socket");
        stopClient(socket);
    }
    //теперь саму тему
    n = write(socket, message, length);
    if (n < 0) {
        perror("ERROR writing to socket");
        stopClient(socket);
    }
}

//Отправка запроса на сервер
void sendRequestToServer(int socket, int flag){

    switch(flag){
        case 1: {
            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n <= 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            break;
        }
        case 2: {
            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n <= 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            break;
        }
        case 3: {
            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n <= 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            //Отправляем специальность
            writePart(socket, proffesion[0]);
            break;
        }
        case 4: {
            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n <= 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            //Передаем критерии
            //Отправляем название специальности
            writePart(socket, findConditon[0]);
            //Отправляем название компании
            writePart(socket, findConditon[1]);
            //Отправляем зарплату
            writePart(socket, findConditon[2]);
            //Отправляем должность
            writePart(socket, findConditon[3]);
            //Отправляем возраст
            writePart(socket, findConditon[4]);
            
            break;
        }
        case 5: {
            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            //Отправляем название специальности
            writePart(socket, proffesion[0]);
            break;
        }
        case 6: {
            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            //Отправляем название специальности
            writePart(socket, massOffers[0]->proff);
            //Отправляем название должности
            writePart(socket, massOffers[0]->positon);
            //Отправляем зарплату
            n = write(socket, &massOffers[0]->sall, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            //Отправляем возраст
            n = write(socket, &massOffers[0]->age, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            //Отправляем название компании
            writePart(socket, massOffers[0]->company);
            break;
        }
    }

}
//Получаем ответ от сервера
void getResponseFromServer(int socket){
    
    int flag = 0;
    int n;
    char *bufferData;
    char *response;
    //Получаем значение флага
    n = read(socket, &flag, sizeof(int));
    
    if (n <= 0 ) {
        perror("ERROR reading from socket\n");
        stopClient(socket);
    }

    switch(flag){
        case 1: {
            //Получаем список вакансий 
            //Количество вакансий
            countOffer = 0;
            n = read(socket, &countOffer, sizeof(int));
            if (n < 0) {
                perror("ERROR reading from socket\n");
                stopClient(socket);
            }
            char * buffer;
            for (int i = 0; i < countOffer; i++){
                //Считываем вакансию отдельно 
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
                    stopClient(socket);
                }
                newOffer->sall = salary;
                //Получаем возраст
                int age = 0;
                n = read(socket, &age, sizeof(int));
                if (n <= 0) {
                    perror("ERROR reading from socket\n");
                    stopClient(socket);
                }
                newOffer->age = age;
                //Получаем компанию
                buffer = partRead(socket, buffer);
                newOffer->company = strdup(buffer);
                massOffers[i] = newOffer;
                free(buffer);
            }
            break;
        }
        case 2: {
            
            int n;
            //Инициализируем переменную чтобы получить количество профессий с сервера
            int countP = 0;
            //Получили количество
            n = read(socket, &countP, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            for (int i = 0; i < countP; i++){
                bufferData = partRead(socket,bufferData);
                addProff(bufferData);
                free(bufferData);
            }
            break;

        }
        case 3: {
            response = partRead(socket, response);
            printf("%s\n",response);
            free(response);
            break;
        }
        case 4: {
            //Получаем список вакансий 
            //Количество вакансий
            countOffer = 0;
            n = read(socket, &countOffer, sizeof(int));
            if (n < 0) {
                perror("ERROR reading from socket\n");
                stopClient(socket);
            }
            char * buffer;
            for (int i = 0; i < countOffer; i++){
                //Считываем вакансию отдельно 
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
                    stopClient(socket);
                }
                newOffer->sall = salary;
                //Получаем возраст
                int age = 0;
                n = read(socket, &age, sizeof(int));
                if (n <= 0) {
                    perror("ERROR reading from socket\n");
                    stopClient(socket);
                }
                newOffer->age = age;
                //Получаем компанию
                buffer = partRead(socket, buffer);
                newOffer->company = strdup(buffer);
                massOffers[i] = newOffer;
                free(buffer);
            }
            break;
        }
        case 5: {
            response = partRead(socket, response);
            printf("%s\n",response);
            free(response);
            break;
        }
        case 6:{
            response = partRead(socket, response);
            printf("%s\n",response);
            free(response);
            break;
        }
    }

}

//argv[2] - host; argv[3] - port
int main(int argc, char *argv[]) {

    int sockfd, n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //Структуры для изменения режима работы терминала
    struct termios initial_settings, new_settings;
    //Контролирует нажатую кнопку в терминале
    char pressButton;
    //Начальное состояние консоли
    tcgetattr(fileno(stdin), &initial_settings);

    //Инициализируем массивы
    initProffesion();
    initCond();
    
    //Проверка на корректность введенных данных
    if (argc < 2) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    //Инициализция номера порта
    portno = (uint16_t) atoi(argv[2]);
    /*Создание сокета*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Ошибка при открытии сокета");
        stopClient(sockfd);
    }

    printf("Здравствуй! Добро пожаловать в систему по поиску/публикации вакансий\n");
    
    //Инициализируем соединение с сервером
    server = gethostbyname(argv[1]);
    //Проверяем что хост существует и корректный
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }


    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Подключаемся к серверу */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        stopClient(sockfd);
    }
    printf("Для вызова меню используй клавишу -m\nДля тогого чтобы выйти из программы нажмите -q\n");
    //Работа клиента
    while(1){

        new_settings = initial_settings;
        new_settings.c_lflag &= ~ICANON;
        new_settings.c_lflag &= ~ECHO;
        new_settings.c_cc[VMIN] = 0;
        new_settings.c_cc[VTIME] = 0;
        tcsetattr(fileno(stdin), TCSANOW, &new_settings);
        //Ожидаем нажатие клавиши
        read(0, &pressButton, 1);

        //Проверка нажатой клавиши
        if(pressButton == 'm'){

            printf("-1 - Получить список вакансий\n");
            printf("-2 - Получить список специальностей\n");
            printf("-3 - Удалить специальность\n");
            printf("-4 - Поиск вакансии по критериям\n");
            printf("-5 - Добавить специальность\n");
            printf("-6 - Добавить вакансию\n");
            printf("-q - Выйти\n");
        }

        if(pressButton == '1'){
            //Получаем список вакансий
            int flag = 1;
            sendRequestToServer(sockfd,flag);
            getResponseFromServer(sockfd);
            printf("Список вакансий:\n");
            for (int i = 0; i < countOffer; i++){
                printf("-----------ВАКАНСИЯ [%d]----------\n", i);
                printf("[Профессия]: <%s>\n", massOffers[i]->proff);
                printf("[Должность]: <%s>\n", massOffers[i]->positon);
                printf("[Зарплата]: <%d>\n", massOffers[i]->sall); 
                printf("[Возраст]: <%d>\n", massOffers[i]->age); 
                printf("[Компания]: <%s>\n", massOffers[i]->company);
                printf("----------------------------------\n", i);
            }
            for (int i = 0; i < countOffer; i++){
                massOffers[i] = NULL;
            }
            countOffer = 0;
            
        }

        if(pressButton == '2'){
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings);
            //Получаем список специальностей
            //Отправляем запрос на сервер
            int flag = 2;
            sendRequestToServer(sockfd,flag);
            getResponseFromServer(sockfd);
            printf("Список профессий:\n");
            for (int i = 0; i < countProff; i++){
               printf("-Профессия [%d] - <%s>\n", i, proffesion[i]);
            }
            for (int i = 0; i < countProff; i++){
                free(proffesion[i]);
            }
            countProff = 0;
        }

        if(pressButton == '3'){
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings);
            //удаляем специальность
            char buffer[buffMessage];
            bzero(buffer, buffMessage); 
            printf("Какую специальность хотите удалить?: ");
            fgets(buffer, buffMessage, stdin);
            //Убираем последний пробел
            buffer[strlen(buffer) - 1] = 0;
            proffesion[0] = strdup(buffer);
            int flag = 3;
            sendRequestToServer(sockfd,flag);
            getResponseFromServer(sockfd);
            free(proffesion[0]);
        }

        if(pressButton == '4') {
            //Поиск
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings);
            printf("По какому /каким критериям ищите ? \n");
            printf("-------Список критериев------------\n");
            printf("- Специальность\n");
            printf("- Компания\n");
            printf("- Зарплата\n");
            printf("- Должность\n");
            printf("- Возраст\n");
            printf("-------------------------------------\n");
            char buffer[buffMessage];
            bzero(buffer, buffMessage); 
            printf("Введите название специальности: ");
            fgets(buffer, buffMessage, stdin);
            buffer[strlen(buffer) - 1] = 0;
            findConditon[0] = strdup(buffer);
            bzero(buffer, buffMessage); 
            printf("Введите название компании: ");
            fgets(buffer, buffMessage, stdin);
            buffer[strlen(buffer) - 1] = 0;
            findConditon[1] = strdup(buffer);
            bzero(buffer, buffMessage); 
            printf("Введите зарплату в тысячах р.: ");
            fgets(buffer, buffMessage, stdin);
            buffer[strlen(buffer) - 1] = 0;
            findConditon[2] = strdup(buffer); 
            bzero(buffer, buffMessage); 
            printf("Введите должность: ");
            fgets(buffer, buffMessage, stdin);
            buffer[strlen(buffer) - 1] = 0;
            findConditon[3] = strdup(buffer);
            bzero(buffer, buffMessage); 
            printf("Введите возраст: ");
            fgets(buffer, buffMessage, stdin);
            buffer[strlen(buffer) - 1] = 0;
            findConditon[4] = strdup(buffer);
            int flag = 4;
            sendRequestToServer(sockfd,flag);
            getResponseFromServer(sockfd);
            for(int i = 0; i < maxCond; i++){
                free(findConditon[i]);
            }
            printf("Список вакансий:\n");
            for (int i = 0; i < countOffer; i++){
                printf("-----------ВАКАНСИЯ [%d]----------\n", i);
                printf("[Профессия]: <%s>\n", massOffers[i]->proff);
                printf("[Должность]: <%s>\n", massOffers[i]->positon);
                printf("[Зарплата]: <%d>\n", massOffers[i]->sall); 
                printf("[Возраст]: <%d>\n", massOffers[i]->age); 
                printf("[Компания]: <%s>\n", massOffers[i]->company);
                printf("----------------------------------\n", i);
            }
            for (int i = 0; i < countOffer; i++){
                massOffers[i] = NULL;
            }
            countOffer = 0;
        }

        if(pressButton == '5'){
            //Обработка создания специальности
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings);
            // news* nNews = (news *) malloc (sizeof(news));
            char buffer[buffMessage];
            bzero(buffer, buffMessage); 
            printf("Введите название специальности: ");
            fgets(buffer, buffMessage, stdin);
            //Убираем последний пробел
            buffer[strlen(buffer) - 1] = 0;
            proffesion[0] = strdup(buffer);
            int flag = 5;
            sendRequestToServer(sockfd,flag);
            getResponseFromServer(sockfd);
            free(proffesion[0]);
        }
        if(pressButton == '6'){
            //Обработка создания вакансии
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings);
            offers* newOffer = (offers *) malloc (sizeof(offers));
            char buffer[buffMessage];
            bzero(buffer, buffMessage); 
            printf("Введите название специальности: ");
            fgets(buffer, buffMessage, stdin);
            //Убираем последний пробел
            buffer[strlen(buffer) - 1] = 0;
            newOffer->proff = strdup(buffer);
            bzero(buffer, buffMessage); 
            printf("Введите название компании: ");
            fgets(buffer, buffMessage, stdin);
            buffer[strlen(buffer) - 1] = 0;
            newOffer->company = strdup(buffer);
            bzero(buffer, buffMessage); 
            printf("Введите зарплату в тысячах р.: ");
            fgets(buffer, buffMessage, stdin);
            buffer[strlen(buffer) - 1] = 0;
            newOffer->sall = (int) atoi(buffer); 
            bzero(buffer, buffMessage); 
            printf("Введите должность: ");
            fgets(buffer, buffMessage, stdin);
            buffer[strlen(buffer) - 1] = 0;
            newOffer->positon = strdup(buffer);
            bzero(buffer, buffMessage); 
            printf("Введите возраст: ");
            fgets(buffer, buffMessage, stdin);
            buffer[strlen(buffer) - 1] = 0;
            newOffer->age = (int) atoi(buffer);
            massOffers[countOffer] = newOffer;
            int flag = 6;
            sendRequestToServer(sockfd,flag);
            getResponseFromServer(sockfd);
            massOffers[countOffer] = NULL;
        }
        if(pressButton == 'q'){
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings);
            stopClient(sockfd);
        }

        //Сбрасываем кнопку нажатия сообщения
        pressButton = 0;
    }

    return 0;
}
