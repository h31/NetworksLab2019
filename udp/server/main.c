#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <termios.h>
#include <signal.h>


#define port 5004
#define maxClients 10
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
//Определяет тип пакета
uint16_t flag;
int sockfd;
char buffer[dataSize];
int clientNumber = 0;
//Файл с которым будем работать
FILE *file;
//Длина принятого пакет
int lengthPacket;
char *nameFile;
int blockNumber;
//Пакет от сервера
char packetFROM[packetSizeTFTP];
//Пакет серверу
char packetTO[packetSizeTFTP];
//Размер исходящего пакета
int outLengthPacket;
struct sockaddr_in serv_addr, cli_addr;
char *messErr;
int countREAD;
//Структура клиента
typedef struct{
    //Храним адрес клиента
    struct sockaddr_in addr;
    //Файл с которым он работает
    FILE *file;
    //Номер блока блока переданного клиенту
    uint16_t numberBLOCK;

}client;

//Массив клиентов
client clients[maxClients];

//Функция закрытия сервера
void closeServer(){
    close(sockfd);
    exit(1);
}
int numberError = 0;
//Функция формирователь пакетов
void makePacket(){

    switch (flag){
        //Формируем пакет данных
        case flagDATA : {
            bzero(packetTO, packetSizeTFTP);
            //2 байта на тип пакета
            flag = htons(flag);
            memcpy(packetTO,  &flag, 2);
            //2 байта на номер блока
            blockNumber = htons(blockNumber);
            memcpy(packetTO + 2, &blockNumber, 2);
            //N байт на данные
            memcpy(packetTO + 4 , buffer , strlen(buffer));
            //Формируем пакет
            break;
        }
            //Пакет подтверждения принятия / отправки данных
        case flagACK : {
            bzero(packetTO, packetSizeTFTP);
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
            //2 байта на тип пакета
            flag = htons(flag);
            memcpy(packetTO,  &flag, 2);
            //2 байта на номер блока
            numberError = htons(numberError);
            memcpy(packetTO + 2,  &blockNumber, 2);
            memcpy(packetTO + 4,  messErr, strlen(messErr) +1);
            //Формируем пакет
            break;
        }
    }

}

//Отправка данных клиенту
void sendPacketToClient(int length){
    int lengthSend = sendto(sockfd, (char *) packetTO, length, 0, (struct sockaddr  *) &cli_addr, sizeof(cli_addr));
    if (lengthSend < 0){
        printf("ERROR");
        closeServer();
    }
}

//Записываем клиента в массив
void setClient(){
    //Подключаем нового клиента
    for( int i = 0; i < maxClients; i++){
        if (clients[i].addr.sin_addr.s_addr == 0){
            clientNumber = i;
            break;
        }
    }

    //Добавляем нового клиента
    clients[clientNumber].addr = cli_addr;
}

//Ищем клиента в массиве
void findClient(){
    for( int i = 0; i < maxClients; i++){
        if (clients[i].addr.sin_addr.s_addr == cli_addr.sin_addr.s_addr){
            clientNumber = i;
            break;
        }
    }
}

//Обработка сигнала выхода от пользователя
void signalExit(int sig){
    closeServer();
}

int main(int argc, char *argv[]) {

    uint16_t portno;
    unsigned int clilen;
    ssize_t n;

    signal(SIGINT, signalExit);

    /*Сокет для прослушивания других клиентов */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

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

    //Работа сервера
    while (1){
        //Принимаем очередной пакет от клиента
        socklen_t size  = sizeof(serv_addr);
        lengthPacket = recvfrom(sockfd, (char *) packetFROM, packetSizeTFTP, 0, (struct sockaddr  *) &cli_addr, &size);

        flag = ntohs(*(uint16_t *) packetFROM);
        switch(flag){
            case flagWRITE : {
                setClient();
                //считываем имя файла
                nameFile = packetFROM + 2;
                //Проверка на существование
                file = fopen(nameFile, "r");
                if (file == NULL){
                    //Если NULL - все хорошо такого файла на сервере еще нет можем принимать
                    file = fopen(nameFile, "w");
                    blockNumber = 0;
                    flag = flagACK;
                    clients[clientNumber].file = file;
                    clients[clientNumber].numberBLOCK = 0;
                    //Формируем первый пакет на принятие файла

                    makePacket();
                    outLengthPacket = 4;
                    sendPacketToClient(outLengthPacket);
                }
                else{
                    fclose(file);
                    flag = flagERROR;
                    messErr = "ФАЙЛ СУЩЕСТВУЕТ";
                    numberError = 1;
                    makePacket();
                    outLengthPacket = 4 + strlen(messErr) + 1;
                    sendPacketToClient(outLengthPacket);
                    //Файл на сервере существует отправляем ошибку
                }

                break;
            }
            case flagREAD : {
                setClient();
                //считываем имя файла
                nameFile = packetFROM + 2;
                //Проверка на существование
                file = fopen(nameFile, "r");
                if (file == NULL){
                    flag = flagERROR;
                    messErr = "ФАЙЛ НЕ СУЩЕСТВУЕТ";
                    numberError = 2;
                    makePacket();
                    outLengthPacket = 4 + strlen(messErr) + 1;
                    sendPacketToClient(outLengthPacket);
                }
                else{
                    //Файл существует начинаем отправлять файл
                    clients[clientNumber].file = file;
                    clients[clientNumber].numberBLOCK = 0;
                    bzero(buffer, dataSize);
                    countREAD = fread(buffer, sizeof(char), dataSize, clients[clientNumber].file);
                    flag = flagDATA;
                    clients[clientNumber].numberBLOCK = clients[clientNumber].numberBLOCK + 1;
                    blockNumber = clients[clientNumber].numberBLOCK;
                    outLengthPacket = countREAD + 4;
                    makePacket();
                    sendPacketToClient(outLengthPacket);
                }

                break;
            }
            case flagDATA :{
                //Проверяем клиента
                findClient();
                //Читаем содержимое пакета
                blockNumber = ntohs(*(uint16_t *) packetFROM + 2);

                if (clientNumber < maxClients){

                    //Записываем данные в файл
                    fwrite(packetFROM + 4, sizeof(char), lengthPacket - 4, clients[clientNumber].file);
                    flag = flagACK;
                    makePacket();
                    outLengthPacket = 4;
                    sendPacketToClient(outLengthPacket);

                    if (lengthPacket < dataSize){
                        //Конец приема файла
                        fclose(clients[clientNumber].file);
                        bzero(&clients[clientNumber].addr, sizeof(clients[clientNumber].addr));
                    }

                }
                else{
                    printf("Unknow client\n");
                }

                break;
            }
            case flagACK :{
                if (clients[clientNumber].numberBLOCK < 0){
                    fclose(clients[clientNumber].file);
                    bzero(&clients[clientNumber].addr, sizeof(clients[clientNumber].addr));
                }
                else{
                    bzero(buffer, dataSize);
                    countREAD = fread(buffer, sizeof(char), dataSize, clients[clientNumber].file);
                    flag = flagDATA;
                    clients[clientNumber].numberBLOCK = clients[clientNumber].numberBLOCK + 1;
                    blockNumber = clients[clientNumber].numberBLOCK;
                    outLengthPacket = countREAD + 4;
                    makePacket();
                    sendPacketToClient(outLengthPacket);
                    if (countREAD < dataSize){
                        clients[clientNumber].numberBLOCK = -1;
                    }
                }
                break;
            }
        }
    }


    return 0;
}