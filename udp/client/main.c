#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>


#define flagWRITE 1
#define flagREAD 2
#define flagDATA 3
#define flagACK 4
#define flagERROR 5
//512 байт данных
#define dataSize 512
//516 байт пакета tftp
#define packetSizeTFTP 516
#define sizeNameFile 64

//Режим отправки строк заканчивающихся /0
char mode[] = "octet";
//Буфер для чтения данных из файла
char buffer[dataSize];
//Определяет тип пакета 
uint16_t flag = 0;
//Номер блока
uint16_t blockNumber = 0;
//Файл с которым будем работать
FILE *file;
//Длина принятого пакет
int lengthPacket;
//Длина отправленного пакета
int outLengthPacket;
//Количество прочитанных символов с файла
int countREAD = dataSize;
//Сокет клиента
int sockfd;
//Имя файла который отправляется
char nameFile[sizeNameFile];
//Пакет от сервера
char packetFROM[packetSizeTFTP];
//Пакет серверу
char packetTO[packetSizeTFTP];

struct sockaddr_in serv_addr;
//Функция закрытия клиента
void stopClient(int socket){
    shutdown(socket,SHUT_RDWR);
    close(socket);
    exit(1);
}

//Функция формирователь пакетов
void makePacket(){
    
    switch (flag){
        //Отправляем серверу запрос где говорим, что будем записывать файл
        case flagWRITE : {
            bzero(packetTO, packetSizeTFTP);
            //2 байта на тип пакета 
            flag = htons(flag);
            memcpy(packetTO, &flag, 2);
            // N байт на имя файла 
            memcpy(packetTO + 2, nameFile, strlen(nameFile) + 1);
            //N байт на mode
            memcpy(packetTO + 2 + strlen(nameFile) + 1,mode, strlen(mode)  + 1);
            //Формируем пакет
            break;
        } 
        //Формируем пакет для чтения файла с сервера
        case flagREAD : {
            bzero(packetTO, packetSizeTFTP);
            //2 байта на тип пакета 
            flag = htons(flag);
            memcpy(packetTO, &flag, 2);
            // N байт на имя файла 
            memcpy(packetTO + 2, nameFile, strlen(nameFile) + 1);
            //N байт на mode
            memcpy(packetTO + 2 + strlen(nameFile) + 1,mode, strlen(mode)  + 1);
            //Формируем пакет
            break;
        }
        //Формируем пакет данных
        case flagDATA : {        
            //2 байта на тип пакета 
            flag = htons(flag);
            memcpy(packetTO,  &flag, 2);
            //2 байта на номер блока
            blockNumber = htons(blockNumber);
            memcpy(packetTO + 2, &blockNumber, 2);
            //N байт на данные
            memcpy(packetTO + 4 , buffer, strlen(buffer));
            //Формируем пакет
            break;
        }
        //Пакет подтверждения принятия / отправки данных
        case flagACK : {
            //2 байта на тип пакета 
            flag = htons(flag);
            memcpy(packetTO,  &flag, 2);
            //2 байта на номер блока
            blockNumber = htons(blockNumber);
            memcpy(packetTO + 2,  &blockNumber, 2);
            //Формируем пакет
            break;
        }
        //Формирование пакета ошибки
        case flagERROR : {

            break;
        }
    }

}

//Отправляем пакет серверу
void sendPacketToServer(int length ){
    sendto(sockfd, (const char *) packetTO, length, 0, (struct sockaddr  *) &serv_addr, sizeof(serv_addr));
}

//Получаем пакет с сервера
void recivePacketFromServer(){
    bzero(packetFROM, packetSizeTFTP);
    socklen_t size  = sizeof(serv_addr);
    lengthPacket = recvfrom(sockfd, (char *) packetFROM, packetSizeTFTP, 0, (struct sockaddr  *) &serv_addr, &size);

    flag = ntohs(*(uint16_t *) packetFROM);
}

void sendFileToServer(){
    //Отправляем пакет инициализации
    sendPacketToServer(sizeNameFile);

    while (1){
        //Получаем очередной пакет
        recivePacketFromServer();
        if (countREAD != dataSize){
            break;
        }
        switch(flag){
            
            case flagACK : {
                blockNumber = ntohs(*(uint16_t *) packetFROM + 2);
                bzero(buffer, dataSize);
                countREAD = fread(buffer, sizeof(char), dataSize, file);
                flag = flagDATA;
                blockNumber = blockNumber + 1;
                outLengthPacket = countREAD + 4;
                makePacket();
                sendPacketToServer(outLengthPacket);
                break;
            }

            case flagERROR : {
                printf("ERROR %s\n", packetFROM + 4);
                break;
            }
        }

    }

    countREAD = dataSize;
    if (flag != flagERROR){
        printf("Файл передан\n");
    }
    fclose(file);
}


void reciveFileFromServer(){
    //Отправляем пакет инициализации
    sendPacketToServer(packetSizeTFTP);
    
    while (1){
        //Получаем очередной пакет
        recivePacketFromServer();
        switch(flag){
            
            case flagDATA: {
                blockNumber = ntohs(*(uint16_t *) packetFROM + 2);
                //Записываем данные в файл
                fwrite(packetFROM + 4, sizeof(char), lengthPacket - 4, file);
                flag = flagACK;
                makePacket();
                outLengthPacket = 4;
                sendPacketToServer(outLengthPacket);
                break;
            }
            case flagERROR : {
                printf("ERROR %s\n", packetFROM + 4);

                break;
            }
        }
        if (lengthPacket != packetSizeTFTP){
            break;
        }
    }

    if (flag != flagERROR{
        printf("Файл скачан\n");
    }
    fclose(file);

}

int main(int argc, char *argv[]) {

    int n;
    uint16_t portno;
    struct hostent *server;

    //Структуры для изменения режима работы терминала
    struct termios initial_settings, new_settings;
    //Контролирует нажатую кнопку в терминале
    char pressButton;
    //Начальное состояние консоли
    tcgetattr(fileno(stdin), &initial_settings);

    if (argc < 2){
        printf("Недостаточное число аргументов\n");
    }

    portno = (uint16_t) atoi(argv[2]);

     //Инициализируем соединение с сервером
    server = gethostbyname(argv[1]);

    //Проверяем что хост существует и корректный
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Ошибка при открытии сокета");
        stopClient(sockfd);
    }

    //Инициализируем настройки клиента 
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    //Параметры функции sendto
    /*
    *   int socket
    *   void *msg (Сообщение)
    *   int len - длина сообщения
    *   flags - флаги
    *   sockaddr *to - кому отправлять 
    *   int tolen
    */
    //Параметры функции reciveto
    /*
    *   int socket
    *   void *buf (Сообщение)
    *   int len - длина сообщения
    *   flags - флаги
    *   sockaddr *from - от кого получили
    *   int fromlen
    */

    printf("Здравсвтуй! Добро пожаловать в клиент tftp. Для отправки файла нажми -1\n Для скачивания нажми -2\n");
    
    while(1){

        new_settings = initial_settings;
        new_settings.c_lflag &= ~ICANON;
        new_settings.c_lflag &= ~ECHO;
        new_settings.c_cc[VMIN] = 0;
        new_settings.c_cc[VTIME] = 0;

        tcsetattr(fileno(stdin), TCSANOW, &new_settings);
        //Ожидаем нажатие клавиши
        read(0, &pressButton, 1);
        if (pressButton == '1'){
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings); 
            bzero(nameFile, sizeNameFile); 
            printf("Введите название файла: ");
            fgets(nameFile, sizeNameFile, stdin);
            nameFile[strlen(nameFile) - 1] = 0;
            //Открываем файл на чтение 
            file = fopen(nameFile, "r");
            //Проверяем существует ли файл 
            if (file == NULL){
                printf("Ошибка при открытии файла\n");
            }
            else{
                flag = flagWRITE;
                makePacket();
                sendFileToServer();
            }
        }
        if (pressButton == '2'){
            
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings); 
            bzero(nameFile, sizeNameFile); 
            printf("Введите название файла: ");
            fgets(nameFile, sizeNameFile, stdin);
            nameFile[strlen(nameFile) - 1] = 0;
            file = fopen(nameFile, "w");
            flag = flagREAD;
            makePacket();
            reciveFileFromServer();

        }
        
        pressButton = 0;

    }

    close(sockfd);
    return 0;
}
