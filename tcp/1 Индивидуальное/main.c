#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>

//Максимальная длина сообщения
#define buffMessage 512
//Максимальная длина темы
#define sizeTheme 128
//Максимальное количество записей новостей
#define maxNews 1024

//Структура описывающая новость
typedef struct{
    char *name; // Имя новости
    char *textNews; // текст новости
    char *themes; // К какой теме принадлежит новость
} news;

//Начальное значение количества тем
int countThemes = 100;
//Счетчик тем, для обработки заполнености массива тем
int countT = 0;
//Счетчик новостей 
int countNews = 0;
//Массив тем
char **themes;
//Массив названий новостей
char **newsName;
//Тема по которой ищем новость
char *tempThemes;
//Имя по которой полкчаем новость
char *tempName;
//Массив новостей
news* tempNews;
news *massNews[maxNews];

//Функция закрытия клиента
void stopClient(int socket){
    shutdown(socket,SHUT_RDWR);
    close(socket);
    exit(1);
}

void initMassiveTheme(){
    themes = malloc(countThemes*sizeof(void *));
    for (int i = 0; i < countThemes; i++){
        themes[i] = (char *) malloc (sizeTheme);
    }
}

void initMassiveNews(){
    newsName = malloc(countThemes*sizeof(void *));
    for (int i = 0; i < countThemes; i++){
        newsName[i] = (char *) malloc (sizeTheme);
    }
}

//Добавление темы
void addThemes(char* theme){

    if (countT != countThemes){
        themes[countT] = strdup(theme);
        countT++;
    }

}

//Добавление новости
void addNews(char* name){
    newsName[countNews] = strdup(name);
    countNews++;
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
            writePart(socket, tempThemes);
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
            writePart(socket, tempName);
            break;
        }
        case 4: {
            int n;
            //Отправили флаг
            n = write(socket, &flag, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            //Отправили количество тем
            n = write(socket, &countT, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            for (int i = 0; i < countT; i++){
                //Сначала отправляем размер 
                printf("outputTheme %s\n", themes[i]);
                printf("output length = %d\n", strlen(themes[i]));
                writePart(socket, themes[i]);                     
            }
            printf("Отправлено\n");
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
            //Сначала отправляем тему
            writePart(socket,massNews[0]->themes);
            //Отправка заголовка 
            writePart(socket,massNews[0]->name);
            //Отправка текста
            writePart(socket,massNews[0]->textNews);           
            massNews[0] = NULL;
            printf("Отправлено\n");
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
            int tempCount = 0;
            n = read(socket, &tempCount, sizeof(int));
            if (n < 0) {
                perror("ERROR reading from socket\n");
                stopClient(socket);
            }
            for (int i = 0; i < tempCount; i++){
                bufferData = partRead(socket,bufferData);
                addThemes(bufferData);
                free(bufferData);
            }
            break;
        }
        case 2: {
            
            int n;
            int countNew = 0;
            //Отправили количество новостей
            n = read(socket, &countNew, sizeof(int));
            if (n < 0) {
                perror("ERROR writing to socket");
                stopClient(socket);
            }
            for (int i = 0; i < countNew; i++){
                bufferData = partRead(socket,bufferData);
                addNews(bufferData);
                free(bufferData);
            }
            break;
        }
        case 3: {
            tempNews = (news *) malloc (sizeof(news));
            char *buffer; 
            //Получаем тему
            buffer = partRead(socket, buffer);
            tempNews->themes = strdup(buffer);
            //Получаем заголовок
            buffer = partRead(socket, buffer);
            tempNews->name = strdup(buffer);
            //Получаем текст
            buffer = partRead(socket, buffer);
            printf("teex %s\n", buffer);
            tempNews->textNews = strdup(buffer);
            free(buffer);
            break;
        }
        case 4: {
            response = partRead(socket, response);
            printf("%s\n",response);
            free(response);
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

//Вывод новости на экран
void printNews(news* news){
    printf("Тема: %s\n", news->themes);
    printf("Название: %s\n", news->name);
    printf("Текст: %s\n", news->textNews);
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

    //Инициализируем массив тем
    initMassiveTheme();
    initMassiveNews();
   
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

    printf("Здравствуй! Добро пожаловать в приложению по поиску публикации/получения новостей\n");
    
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
            printf("-1 - Получить список тем\n");
            printf("-2 - Получить список новостей по теме\n");
            printf("-3 - Получить конкретную новость\n");
            printf("-4 - Опубликовать тему\n");
            printf("-5 - Опубликовать новость\n");
            printf("-q - Выйти\n");
        }

        //Получаем список тем
        if(pressButton == '1'){
            
            int flag = 1;
            sendRequestToServer(sockfd,flag);
            getResponseFromServer(sockfd);
            printf("Список тем:\n");
            for (int i = 0; i < countT; i++){
                printf("-%d - %s\n", i, themes[i]);
            }

            for (int i = 0; i < countT; i++){
                free(themes[i]);
            }
            countT = 0;
            
        }

        if(pressButton == '2'){
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings);
            //Получаем новости по теме
            char buffer[buffMessage];
            bzero(buffer, buffMessage); 
            printf("Введите тему: ");
            //Получаем название темы
            fgets(buffer, buffMessage, stdin);
            //Убираем последний пробел
            buffer[strlen(buffer) - 1] = 0;
            tempThemes = malloc(strlen(buffer)*sizeof(char));
            tempThemes = strdup(buffer);
            //Отправляем запрос на сервер
            int flag = 2;
            sendRequestToServer(sockfd,flag);
            getResponseFromServer(sockfd);
            printf("Список новостей:\n");
            for (int i = 0; i < countNews; i++){
                printf("-%d - %s\n", i, newsName[i]);
            }
            for (int i = 0; i < countNews; i++){
                free(newsName[i]);
            }
            free(tempThemes);
            countNews = 0;
        }

        if(pressButton == '3'){
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings);
            //Получаем конкретную новость
            char buffer[buffMessage];
            bzero(buffer, buffMessage); 
            printf("Введите новость: ");
            //Получаем название новости
            fgets(buffer, buffMessage, stdin);
            //Убираем последний пробел
            buffer[strlen(buffer) - 1] = 0;
            tempName = malloc(strlen(buffer)*sizeof(char));
            tempName = strdup(buffer);
            //Отправляем запрос на сервер
            int flag = 3;
            sendRequestToServer(sockfd,flag);
            getResponseFromServer(sockfd);
            if (tempNews != NULL){
                printf("Полученная новость \n");
                printNews(tempNews);
            }
            tempNews = NULL;
            free(tempName);
        }

        if(pressButton == '4') {
            int nextTheme = 1;
            //Восстанавливаем исходный терминал
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings);
            while (nextTheme) {
                 //Буфер введенных пользователем данных
                char buffer[buffMessage];
                bzero(buffer, buffMessage); 
                printf("Введите тему: ");
                //Получаем название темы
                fgets(buffer, buffMessage, stdin);
                //Убираем последний пробел
                buffer[strlen(buffer) - 1] = 0;
                addThemes(buffer);

                printf("Хотите добавить еще тему y/n?: ");
                //Ожидаем нажатие клавиши
                scanf("%c", &pressButton);
                if (pressButton == 'y') {
                    nextTheme = 1;
                    pressButton = 0;
                } else {
                    nextTheme = 0;
                }
                getchar();
            }
            printf("Количество тем %d\n", countT);
            int flag = 4;
            sendRequestToServer(sockfd, flag);
            getResponseFromServer(sockfd);
            for (int i = 0; i < countT; i++){
                free(themes[i]);
            }
            countT = 0;
        }

        if(pressButton == '5'){
            //Обработка создания новости
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings);
            news* nNews = (news *) malloc (sizeof(news));
            char buffer[buffMessage];
            bzero(buffer, buffMessage); 
            printf("Введите название новости: ");
            fgets(buffer, buffMessage, stdin);
            //Убираем последний пробел
            buffer[strlen(buffer) - 1] = 0;
            nNews->name = strdup(buffer);
            bzero(buffer, buffMessage); 
            printf("Введите текст новости: ");
            fgets(buffer, buffMessage, stdin);
            buffer[strlen(buffer) - 1] = 0;
            nNews->textNews = strdup(buffer);
            bzero(buffer, buffMessage); 
            printf("Введите к какой теме новость принадлежит: ");
            fgets(buffer, buffMessage, stdin);
            buffer[strlen(buffer) - 1] = 0;
            nNews->themes = strdup(buffer);   
            //прочитали - удалили
            massNews[countNews] = nNews;
            int flag = 5;
            sendRequestToServer(sockfd,flag);
            printNews(nNews);
            massNews[countNews] = NULL;
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
