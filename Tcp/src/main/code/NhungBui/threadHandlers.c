#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h> 
#include <math.h> 
#include "course.h"

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
				char tempCourseC[TEMPC_SIZE];
				bzero(tempCourseC, TEMPC_SIZE);
				
				int ID = parseDataFromCmd(4, msg_buf);
				if (ID < 0)
				{
					sendClientConfirm(sock, false);//Ошибка. Валюты с таким ID уже присутствует!
					continue;
				}
				//Заполнение массива ]
				FData[ID].FID = ID;
				FData[ID].Course = FData[ID].conCourse;
				FData[ID].absCourse = 0;
				FData[ID].conCourse = 0;
				FData[ID].HistoryCounter = 0;
				sprintf(tempCourseC, "%.2f", FData[ID].Course);
				strcpy(FData[ID].FHistory[FData[ID].HistoryCounter], tempCourseC);
				sendClientConfirm(sock, true);
				break;		
			}
			
			case 'D':
			case 'd':
			{
				int ID = parseDataFromCmd(1, msg_buf);
				if (ID < 0)
				{
					sendClientConfirm(sock, false);//Ошибка. Валюты с таким ID уже присутствует!
					continue;
				}
				FData[ID].FID = 0x00;
				memset(FData[ID].cname, 0x00, CNAME_SIZE);
				memset(FData[ID].name, 0x00, FNAME_SIZE);
				FData[ID].Course = 0x00;
				FData[ID].absCourse = 0x00;
				FData[ID].conCourse = 0x00;
				sendClientConfirm(sock, true);				
				break;
			}
			case 'C':
			case 'c':
			{	
				char tempCourseD[TEMPC_SIZE];
				memset(tempCourseD, 0x00, TEMPC_SIZE);
				
				int ID = parseDataFromCmd(2, msg_buf);
				if (ID < 0)
				{
					sendClientConfirm(sock, false);//Ошибка. Валюты с таким ID уже присутствует!
					continue;
				}
				float oldCourse = FData[ID].Course;
				FData[ID].Course = FData[ID].conCourse;
				FData[ID].conCourse = FData[ID].Course - oldCourse;
				FData[ID].absCourse = fabs(FData[ID].conCourse);
				if(FData[ID].HistoryCounter >= HIS_NUM)
				{
					FData[ID].HistoryCounter = 0;
				}
				else
				{
					FData[ID].HistoryCounter ++;
				}
				
				sprintf(tempCourseD, "%.2f", FData[ID].Course);
				strcpy(FData[ID].FHistory[FData[ID].HistoryCounter], tempCourseD);
				sendClientConfirm(sock, true);
				break;		
			}
			
			case 'R':
			case 'r':
			{
				sendData(0, 'r', sock);
				break;
			}
			
			case 'H':
			case 'h':
			{
				int ID = parseDataFromCmd(1, msg_buf);
				if (ID < 0)
				{
					sendClientConfirm(sock, false);//Ошибка. Валюты с таким ID уже присутствует!
					continue;
				}
				sendData(ID, 'h', sock);
				break;
			}
		}       
	}
}

void killClient(pthread_t tid, int server)
{
	for (;;) 
	{
		char text[msg_length];
		char com[msg_length];
		bzero(text, msg_length);
		bzero(com, msg_length);
		
		fgets(text, msg_length, stdin);
		text[strlen(text) - 1] = '\0';
		
		if (!strCmpWithoutSpaces(text, EXIT)) 
		{	
			printf("Goodbye!\n");
			pthread_cancel(tid);
			strcpy(text, "/goodbye");
			
			pthread_mutex_lock(&mp);
			for (int i = 0; i < max_clients; i++) 
			{
				if (clients_sockets[i] != 0)
				{
					if (sendFix(clients_sockets[i], text) <= 0) break ;
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
		
		if (!strCmpWithoutSpaces(text, "/k"))
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
					if (sendFix(closed_sock, text) <= 0) break;
					close_client(closed_sock);
				}
			}
		}
	}
}
