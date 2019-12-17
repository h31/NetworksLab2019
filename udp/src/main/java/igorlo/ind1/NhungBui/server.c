#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h> 
#include <math.h> 

#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"
#define how_shutdown 2 	//зарпещена передача данных
#define msg_length 256
#define max_clients 50
#define EXIT "/exit"
#define CNAME_SIZE 30	//название страны
#define FNAME_SIZE 3		//название валют
#define MAX_FID 30
#define HIS_NUM 10
#define TEMPID_SIZE 5
#define TEMPC_SIZE 21

int clients_sockets[max_clients];
pthread_t clients_threads[max_clients];
pthread_mutex_t mp;

struct Finance_data
{
	int FID; 									//ID валюты
	char cname[CNAME_SIZE];	//название страны
	char name[FNAME_SIZE];	// имя валюты
	float Course; 							//Курс валюты
	float conCourse; 					//Относительное приращение курса
	float absCourse; 					//Абсолютное приращение курса
	int HistoryCounter;
	char FHistory[HIS_NUM][21];
};

struct Finance_data FData[MAX_FID];//Массив из структур типа Finance_data

int readN(int sock, char* buf, int length, int flags);
int readFix(int sock, char* buf, int bufSize, int flags);
int sendFix(int sock, char* buf, int flags);
int strCmp(char* static_str, char* final_str);
void SOK(int my_sock, bool OK);
void close_client(int sock); 
int alive_client (int sock);
void* clientHandler(void* arg);
void* acceptHandler(void* arg)
{
	int server = (int) arg;
    struct sockaddr_in cli_addr;
	unsigned int clilen = sizeof(cli_addr);
	int cli_sock;
	int res;
	for (;;) 
	{
		int free_pos = -1;
		pthread_mutex_lock(&mp);
        for (int i = 0; i < max_clients; i++) 
		{
			if (clients_sockets[i] == 0) 
			{
				free_pos = i;
				break;
			}
		}
		pthread_mutex_unlock(&mp);
		
		if (free_pos == -1) 
		{
			continue;
		}
		
		cli_sock = accept(server, (struct sockaddr*) &cli_addr, &clilen);
		if (cli_sock < 0) 
		{
			perror("ERROR on accept");
			exit(1);
		}
		
		pthread_mutex_lock(&mp);
		clients_sockets[free_pos] = cli_sock;
		pthread_mutex_unlock(&mp);	
		
		printf("\nMESSAGE: New client connected!\n"
                "ip = %s; port = %d\n", inet_ntoa(cli_addr.sin_addr), cli_addr.sin_port);
        printf("socket : %d\n\n", cli_sock);      
		
        res = pthread_create(&clients_threads[free_pos], NULL, clientHandler, (void*)(cli_sock));
        if (res) 
		{
            printf("Error while creating new thread\n");
        }
		
    }

}

int main(int argc, char** argv)
{
    
    for (int i = 0; i < max_clients; i++) 
	{
        clients_sockets[i] = 0;
    }

    int port = 0;
    if (argc < 2)
	{
        printf("Using default port %d\n", DEF_PORT);
        port = DEF_PORT;
    }
    else
        port = atoi(argv[1]);
	
	if (pthread_mutex_init(&mp, NULL) != 0) 
	{
		perror("ERROR on mutex:");
		exit(1);
	}
	
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) 
	{
        perror("Can't create socket to listen: ");
        exit(1);
    }

    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) 
	{
        perror("ERROR on setsockopt");
        shutdown(server, how_shutdown);
        close(server);
    }
	
	printf ("Enter <%s> to quit or </k <int>> to kill user with id <int>\n", EXIT);


    int res = bind(server, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (res < 0) 
	{
        perror("Can't bind socket");
        shutdown(server, how_shutdown);
        close(server);
        exit(1);
    }
    // слушаем входящие соединения
    res = listen(server, max_clients);
    if (res) 
	{
        perror("Error while listening:");
        exit(1);
    }
    puts("Waiting for incoming connections");
    // основной цикл работы
	
	pthread_t tid;
	res = pthread_create(&tid, NULL, acceptHandler, (void*) server);
	
	for (;;) 
	{
		char text[msg_length];
		char com[msg_length];
		bzero(text, msg_length);
		bzero(com, msg_length);
		
		fgets(text, msg_length, stdin);
		text[strlen(text) - 1] = '\0';
		
		if (!strCmp(text, EXIT)) 
		{	
			printf("Goodbye!\n");
			pthread_cancel(tid);
			strcpy(text, "/goodbye");
			
	        pthread_mutex_lock(&mp);
			for (int i = 0; i < max_clients; i++) 
			{
				if (clients_sockets[i] != 0)
				{
					if (sendFix(clients_sockets[i], text, 0) <= 0) break ;
					close(clients_sockets[i]);
					pthread_cancel(clients_threads[i]);
					clients_sockets[i] = 0;
					clients_threads[i] = 0;
				}
			}
	        pthread_mutex_unlock(&mp);

			close(server);
			exit(1);
		}
		
		if (!strCmp(text, "/k"))
		{
			int closed_sock = 0;
			printf("Socket number..?\n");
			scanf("%s", com);
            if((closed_sock = atoi(com)) == 0)
            {
                printf("wrong type. (e.g. 123)\n");
            }
			else
            {
				if (!alive_client(closed_sock))
					printf("Sorry, no user with such socket number!\n");
				else 
				{
					strcpy(text, "/goodbye");
					if (sendFix(closed_sock, text, 0) <= 0) break;
					close_client(closed_sock);
				}
			}
		}
	}
	
    shutdown(server, how_shutdown);
    close(server);
    return 0;
}

// обработка одного клиента
void* clientHandler(void* arg)
{
	int sock = (int) arg;
    char msg_buf[msg_length];
    int res = 0;
    
    for (;;) 
	{
        bzero(msg_buf, msg_length);
        res = readFix(sock, msg_buf, msg_length, 0);

        if (res <= 0) 
		{
            printf("A client left chat\n");
			close_client(sock);
        }
		
		switch(msg_buf[0])
		{
			case 'A':
			case 'a':
			{
				//Формат передачи: <ID> <Country> <Currency> <Course>

				int i = 2;
				int j = 0;
				float tempCourse;
				char tempAddID[TEMPID_SIZE];
				bzero(tempAddID, TEMPID_SIZE);
				char tempAddname[CNAME_SIZE];
				bzero(tempAddname, CNAME_SIZE);
				char tempAddsname[FNAME_SIZE];
				bzero(tempAddsname, FNAME_SIZE);
				char tempAddCourse[TEMPC_SIZE];
				bzero(tempAddCourse, TEMPC_SIZE);
				char tempCourseC[TEMPC_SIZE];
				bzero(tempCourseC, TEMPC_SIZE);
				
				while (msg_buf[i] != 0x20) //Получение ID
				{
					tempAddID[j] = msg_buf[i];
					i++;
					j++;
				}
				i = i + 1;	//blank space
				j = 0;
				
				while (msg_buf[i] != 0x20) //Получение имени страны
				{
					tempAddname[j] = msg_buf[i];
					i++;
					j++;
				}
				i = i+1;		//blank space
				j = 0;
				while (msg_buf[i] != 0x20)// получение имени валюты
				{
					tempAddsname[j] = msg_buf[i];
					i++;
					j++;
				}
				i = i+1;		//blank space
				j = 0;
				while (msg_buf[i] != 0x00) //получение курса валюты
				{
					tempAddCourse[j] = msg_buf[i];
					i++;
					j++;
				}
				int tempID;
				tempID = atoi(tempAddID);
				for(int x = 0; x<MAX_FID; x++)
				{
					if(FData[x].FID == tempID)
					{
						SOK(sock, false);//Ошибка. Валюты с таким ID уже присутствует!
						tempID = 0;
						break;
					}
				}
				if (tempID == 0) continue;
				
				//Заполнение массива 
			
				FData[tempID].FID = tempID;
				strcpy(FData[tempID].cname, tempAddname);
				strcpy(FData[tempID].name, tempAddsname);
				tempCourse = atof(tempAddCourse);		//convert course to float
				FData[tempID].Course = tempCourse;
				FData[tempID].absCourse = 0;
				FData[tempID].conCourse = 0;
				FData[tempID].HistoryCounter = 0;
				sprintf(tempCourseC, "%.2f", tempCourse);
				strcpy(FData[tempID].FHistory[FData[tempID].HistoryCounter], tempCourseC);
				/*printf("Added Finance Data: \n"
					"ID  Country                       Currency Course	absCource	conCource   \n"
					"%-4d%-30s%-10s%.2f	%.2f		%.2f\n\n", 
					FData[tempID].FID, FData[tempID].cname, FData[tempID].name, 
					FData[tempID].Course, FData[tempID].absCourse, FData[tempID].conCourse);*/
				SOK(sock, true);
				break;
				
			}
			
			case 'D':
			case 'd':
			{
				
				int tempID;
				tempID = 0;
				char ID[TEMPID_SIZE];
				memset(ID, 0, TEMPID_SIZE);
				int i = 2;
				int j = 0;
				while(msg_buf[i] != 0x00)
				{
					ID[j] = msg_buf[i];
					i++;
					j++;
				}
				tempID = atoi(ID);
				//Удаление конкретной валюты:
				for(int x = 0; x < MAX_FID; x++)
				{
					if (FData[x].FID == tempID)
					{
						/*printf("Deleted Finance Data: \n"
							"ID  Country                       Currency Course	absCource	conCource   \n"
							"%-4d%-30s%-10s%.2f	%.2f		%.2f\n\n", 
							FData[tempID].FID, FData[tempID].cname, FData[tempID].name, FData[tempID].Course, FData[tempID].absCourse, FData[tempID].conCourse);
					*/
						FData[tempID].FID = 0x00;
						memset(FData[tempID].cname, 0x00, CNAME_SIZE);
						memset(FData[tempID].name, 0x00, FNAME_SIZE);
						FData[tempID].Course = 0x00;
						FData[tempID].absCourse = 0x00;
						FData[tempID].conCourse = 0x00;
						SOK(sock, true);
						
						tempID = 0;
						break;
					}
				}
				if (tempID != 0)
					SOK(sock, false);
				break;
			}
			case 'C':
			case 'c':
			{
				int tempID = 0;
				float tempCourse = 0;
				float oldCourse = 0;
				char ID[TEMPID_SIZE];
				memset(ID, 0, TEMPID_SIZE);
				char tempCourseC[TEMPC_SIZE];
				memset(tempCourseC, 0x00, TEMPC_SIZE);
				char tempCourseD[TEMPC_SIZE];
				memset(tempCourseD, 0x00, TEMPC_SIZE);
				int i = 2;
				int j = 0;
				while(msg_buf[i] != 0x20)
				{
					ID[j] = msg_buf[i];
					i++;
					j++;
				}
				tempID = atoi(ID);
				i = i + 1;
				j = 0;
				while(msg_buf[i] != 0x00)
				{
					tempCourseC[j] = msg_buf[i];
					i++;
					j++;
				}
				tempCourse = atof(tempCourseC);
				//Изменение курса валюты
				for(int x = 0; x < MAX_FID; x++)
				{
					if (FData[x].FID == tempID)
					{
						oldCourse = FData[tempID].Course;
						FData[tempID].Course = tempCourse;
						FData[tempID].conCourse = tempCourse - oldCourse;
						FData[tempID].absCourse = fabs(FData[tempID].conCourse);
						if(FData[tempID].HistoryCounter >= HIS_NUM)
						{
							FData[tempID].HistoryCounter = 0;
						}
						else
						{
							FData[tempID].HistoryCounter ++;
						}
						
						sprintf(tempCourseD, "%.2f", tempCourse);
						strcpy(FData[tempID].FHistory[FData[tempID].HistoryCounter], tempCourseD);
						/*printf("Updated Finance Data: \n"
							"ID  Country                       Currency Course	absCource	conCource   \n"
							"%-4d%-30s%-10s%.2f	%.2f		%.2f\n\n", 
							FData[tempID].FID, FData[tempID].cname, FData[tempID].name, FData[tempID].Course, 
							FData[tempID].absCourse, FData[tempID].conCourse);*/
						SOK(sock, true);
						tempID = 0;
						break;
					}
				}
				if (tempID != 0)
					SOK(sock, false);
				break;		
			}
			
			case 'R':
			case 'r':
			{
				int ret;
				char sendbuf[msg_length];
				memset(sendbuf, 0x00, msg_length);
				char tempIDC[TEMPID_SIZE];
				memset(tempIDC, 0x00, TEMPID_SIZE);
				char tempCName[CNAME_SIZE];
				memset(tempCName, 0x00, CNAME_SIZE);
				char tempCSName[FNAME_SIZE];
				memset(tempCSName, 0x00, FNAME_SIZE);
				char tempCourse[TEMPC_SIZE];
				memset(tempCourse, 0x00, TEMPC_SIZE);
				char tempConCourse[TEMPC_SIZE];
				memset(tempConCourse, 0x00, TEMPC_SIZE);
				char tempAbsCourse[TEMPC_SIZE];
				memset(tempAbsCourse, 0x00, TEMPC_SIZE);
				
				for (int i = 0; i < MAX_FID; i++)
				{
					if (FData[i].FID == 0x00 || FData[i].FID == 0)
					{
						continue;
					}
					else
					{
						sprintf(tempIDC, "%d", FData[i].FID);
						sprintf(tempCName, "%s", FData[i].cname);
						sprintf(tempCSName, "%s", FData[i].name);
						sprintf(tempCourse, "%.2f", FData[i].Course);
						sprintf(tempConCourse, "%.2f", FData[i].conCourse);
						sprintf(tempAbsCourse, "%.2f", FData[i].absCourse);
						//Заполнение массива для отправки
						strcat(sendbuf, tempIDC);
						strcat(sendbuf, ",");
						strcat(sendbuf, tempCName);
						strcat(sendbuf, ",");						
						strcat(sendbuf, tempCSName);
						strcat(sendbuf, ",");
						strcat(sendbuf, tempCourse);
						strcat(sendbuf, ",");
						strcat(sendbuf, tempConCourse);
						strcat(sendbuf, ",");						
						strcat(sendbuf, tempAbsCourse);
						strcat(sendbuf, "\n");
					}
				}
				ret = sendFix(sock, sendbuf, 0);

				/*if (ret <= 0) break;
				memset(sendbuf, 0x00, msg_length);
				strcpy(sendbuf,".");//Сигнал о том, что передача закончена
				sendFix(sock, sendbuf, 0);*/
				break;
			}
			
			case 'H':
			case 'h':
			{
				char HIDC[TEMPID_SIZE];
				memset(HIDC, 0x00, TEMPID_SIZE);
				int HID = 0, i = 2, j = 0, flag = 0;
				while(msg_buf[i] != 0x00)
				{
					HIDC[j] = msg_buf[i];
					i++;
					j++;
				}
				HID = atoi(HIDC);
				char sendbuf[msg_length];
				memset(sendbuf, 0x00, msg_length);
				int ret;
				for (int j = 0; j < MAX_FID; j++) 
				{
					if (FData[j].FID != HID) continue;
					flag = 1;
					strcpy(sendbuf, FData[HID].FHistory[0]);
					for (int i = 1; i <= FData[HID].HistoryCounter; i++)
					{
						strcat(sendbuf, ",");
						strcat(sendbuf, FData[HID].FHistory[i]);
					}
					break;
				}
				if (!flag) SOK(sock, false);
				else	ret = sendFix(sock, sendbuf, 0);

				//if (ret <= 0) break;
				/*memset(sendbuf, 0x00, msg_length);
				strcpy(sendbuf,".");//Сигнал о том, что передача закончена
				sendFix(sock, sendbuf, 0);*/
				break;
			}
		}
       
    }
}

void SOK(int my_sock, bool OK)
{
	char sendbuf[msg_length];
	bzero(sendbuf, msg_length);
	if(OK == true)
		strcpy(sendbuf, "Operation passed successfully. ");
	else
		strcpy(sendbuf, "Failed.");
	sendFix(my_sock, sendbuf, 0);
}

void close_client(int sock) 
{
	pthread_mutex_lock(&mp);
	for (int i = 0; i < max_clients; i++)
	{
		if(clients_sockets[i] == sock)
		{
			clients_sockets[i] = 0;
			pthread_cancel(clients_threads[i]);
			clients_threads[i] = 0;
			break;
		}		
	}
	pthread_mutex_unlock(&mp);
	close(sock);
}

int alive_client (int sock) 
{
	int res = 0;
	pthread_mutex_lock(&mp);
	for (int i = 0; i < max_clients; i++)
	{
		if (clients_sockets[i] == sock) 
		{
			res = 1;
			break;
		}			
	}
	pthread_mutex_unlock(&mp);
	return res;
}	

int strCmp(char* static_str, char* final_str) {
	int count = 0;
	char noblank_str[msg_length];
	strcpy(noblank_str, static_str);

	for (int i = 0; static_str[i]; i++)
		if (static_str[i] != ' ')
			noblank_str[count++] = noblank_str[i];
	
	noblank_str[count] = '\0';
	
	if (!strncasecmp(noblank_str, final_str, strlen(final_str)) 
		&& strlen(noblank_str) == strlen(final_str)) return 0;
	else return 1;
}

int readN(int sock, char* buf, int length, int flags) {
	int left;
	int read;
	char* ptr;
	
	ptr = buf;
	left = length;
	while (left > 0) {
		if ((read = recv(sock, ptr, left, flags)) <= 0) {
			return -1;
		}
		left -= read;
		ptr += read;
	}
	return (length - left);
}

int readFix(int sock, char* buf, int bufSize, int flags)
{
    // читаем "заголовок" - сколько байт составляет наше сообщение
    unsigned msgLength = 0;
    int res = readN(sock, &msgLength, sizeof(unsigned), flags | MSG_WAITALL);
    if (res <= 0)
        return res;
    if (res > bufSize) {
        printf("Recieved more data, then we can store, exiting\n");
        exit(1);
    }
    // читаем само сообщение
    return readN(sock, buf, msgLength, flags | MSG_WAITALL);
}

int sendFix(int sock, char* buf, int flags)
{
    // шлем число байт в сообщении
    unsigned msgLength = strlen(buf);
    int res = send(sock, &msgLength, sizeof(unsigned), flags);
    if (res <= 0)
        return res;
    res = send(sock, buf, msgLength, flags);
	if (res <= 0) 
	{
		perror("send call failed: ");
		close_client(sock);
	}
	return res;
}
