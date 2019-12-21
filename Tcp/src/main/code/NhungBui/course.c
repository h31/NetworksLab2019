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

int findGivenID (int givenID)
{
	int res = 0;
	for (int i = 0; i < MAX_FID; i++)
	{
		if (FData[i].FID == givenID)
		{
			res = 1;
			break;
		}
	}
	return res;
}

int parseDataFromCmd(int argsNumb, char msg_buf[msg_length]) 
{
	int i = 2, j = 0;
	char tempAddID[TEMPID_SIZE];
	bzero(tempAddID, TEMPID_SIZE);
	char tempAddCourse[TEMPC_SIZE];
	bzero(tempAddCourse, TEMPC_SIZE);
	char sep;
	if (argsNumb == 1) sep = 0x00;
	else sep = ' ';
	while (msg_buf[i] != sep)	//Получение ID
	{
		tempAddID[j] = msg_buf[i];
		i++;
		j++;
	}
	i++;
	j = 0;
	int tempID = atoi(tempAddID);

	if (argsNumb == 4)	// command A(add)
	{
		if (findGivenID(tempID))
			return -1;
	}
	else if (!findGivenID(tempID))
		return -1;
	
	switch(argsNumb)
	{
		case 1:		//cmd D or H
			return tempID;
		case 4:
			while (msg_buf[i] != ' ' ) //Получение имени страны
			{
				FData[tempID].cname[j] = msg_buf[i];
				i++;
				j++;
			}
			i++;		//blank space
			j = 0;
			while (msg_buf[i] != ' ' )// получение имени валюты
			{
				FData[tempID].name[j] = msg_buf[i];
				i++;
				j++;
			}
			i++;		//blank space
			j = 0;
			break;
		default:
			break;			
	}		
	while (msg_buf[i] != 0x00) //получение курса валюты
	{
		tempAddCourse[j] = msg_buf[i];
		i++;
		j++;
	}
	FData[tempID].conCourse = atof(tempAddCourse);	
	return tempID;	
}

void sendData(int ID, char cmd, int sock)
{
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
	
	if (cmd == 'h')
	{
		strcpy(sendbuf, FData[ID].FHistory[0]);
		for (int i = 1; i <= FData[ID].HistoryCounter; i++)
		{
			strcat(sendbuf, ",");
			strcat(sendbuf, FData[ID].FHistory[i]);
		}
	}
	else 
	{
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
	}
	if (strlen(sendbuf) > 0)
		sendFix(sock, sendbuf);
}

void sendClientConfirm(int cli_sock, bool flag)
{
	char sendbuf[msg_length];
	bzero(sendbuf, msg_length);
	if(flag == true)
		strcpy(sendbuf, "Operation passed successfully. ");
	else
		strcpy(sendbuf, "Failed.");
	sendFix(cli_sock, sendbuf);
}

int strCmpWithoutSpaces(char* static_str, char* final_str) 
{
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