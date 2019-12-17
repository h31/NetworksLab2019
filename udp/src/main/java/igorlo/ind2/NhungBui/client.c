#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

#define DEF_PORT 6789
#define DEF_IP "127.0.0.1"
#define msg_length 4096
#define EXIT "/exit"
#define CYN   "\x1B[36m"
#define RESET "\x1B[0m"
#define MIN_NAME 5
#define MAX_NAME 15
#define NUMB_LENGTH 256

pthread_mutex_t mp;

void* sendHandler(void*);
void printMenu();
int checkArgs(char *msg, char *cmd);
void* rcvHandler(void*);
int strWithoutSpacesCmp(char* static_str, char* final_str);
int readN(int sock, char* buf, int length, int flags);
int readFix(int sock, char* buf, int bufSize, int flags);
int sendFix(int sock, char* buf, int flags);

int main(int argc, char** argv)
{
	pthread_t tid1, tid2;
	int thr1, thr2;
    char* addr;
    int port;

    if (argc < 2) {
        printf("Using default addr %s\n", DEF_IP);
        addr = DEF_IP;
    }
    else
        addr = argv[1];
    if (argc < 3) {
        printf("Using default port %d\n", DEF_PORT);
        port = DEF_PORT;
    }
    else
        port = atoi(argv[2]);

    // создаем сокет
    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    peer.sin_addr.s_addr = inet_addr(addr);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Can't create socket\n");
        exit(1);
    }
	
    // присоединяемся к серверу
    int res = connect(sock, (struct sockaddr*)&peer, sizeof(peer));
    if (res) {
        perror("Can't connect to server:");
        exit(1);
    }
	
	thr1 = pthread_create(&tid1, NULL, sendHandler, (void*)sock);
	if (thr1) {
            printf("Error while creating thread1\n");
			exit(1);
        }
	thr2 = pthread_create(&tid2, NULL, rcvHandler, (void*)sock);
	if (thr2) {
            printf("Error while creating thread2\n");
			exit(1);
        }
		
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	
	pthread_cancel(tid1);
	pthread_cancel(tid2);
	
    return 0;
}

void* sendHandler(void *arg)
{
	int sock = (int) arg, sendAuth = 0;
	char msg[msg_length];

	for(;;) 
	{
		bzero(msg, msg_length);		
		fgets(msg, msg_length, stdin);
		msg[strlen(msg) - 1] = '\0';
		int msgLength = strlen(msg);
		
		if (!strWithoutSpacesCmp(msg, EXIT)) {
			printf("Bye-bye\n");
			close(sock);
			exit(1);			
		}	
		if (sendAuth < 2)
		{
			if (msgLength < 5 || msgLength > 15) 
			{
				printf (CYN "Неправильный формат" RESET "\n");
				continue;
			}
			for (int i = 0; i < msgLength; i++)
			{
				if (!isalnum(msg[i]))
				{
					printf (CYN "Неправильный формат" RESET "\n");
					msgLength = 0;
					break;
				}
			}
			if (!msgLength) continue;
			sendAuth++;
		}		
		else 
		{
			if (!strWithoutSpacesCmp(msg, "help")) 
			{
				printMenu();
				continue;
			}
			if(!strncmp(msg, "fact", strlen("fact"))) 
			{
				if (checkArgs(msg, "fact") < 0) continue;		
			}
			else if (!strncmp(msg, "sqrt", strlen("sqrt"))) 
			{	
				if (checkArgs(msg, "sqrt") < 0) continue;
			}
			else if (!strncmp(msg, "say", strlen("say")))
			{
				if (msg[strlen("say")] != ' ')
				{
					printf("Usage: say <message>\n");
					continue;
				}
			}
			else if (!strncmp(msg, "eval", strlen("eval")))
			{
				if (!strWithoutSpacesCmp(msg, "eval"))
				{
					printf("Usage: eval <expression>\n");
					continue;
				}
			}
			else if (strWithoutSpacesCmp(msg, "functions") 
				&& strWithoutSpacesCmp(msg, "history")) 
			{
				printf(CYN "Неизвестная команда" RESET "\n");
				continue;
			}
		}	
		int res = sendFix(sock, msg, 0);
				if (res <= 0) 
				{
					perror("send call failed");
					close(sock);
					pthread_exit(NULL);
					exit(1);
				}	
	}	
}

int checkArgs(char *msg, char *cmd) {
	if (!strWithoutSpacesCmp(msg, cmd))
	{
		printf("Usage: %s <number>\n", cmd);
		return -1;
	}
	if (msg[strlen(cmd)] != ' ')
	{
		printf(CYN "Неизвестная команда" RESET "\n");
		return -1;
	}
				
	int i = strlen(cmd) + 1, j = 0;
	char tempNumb[NUMB_LENGTH];
	int numb = 0;
	memset(tempNumb, 0x00, NUMB_LENGTH);
	
	while (msg[i] != 0x00)
	{
		if (j >= NUMB_LENGTH) 
		{
			printf("Number is too long!\n");
			numb = -1;
			break;
		}
		tempNumb[j] = msg[i];
		i++;
		j++;
	}

	if (numb == -1) return numb;
	if (atoi(tempNumb) <= 0) 
	{
		printf("Usage: %s <positive number>\n", cmd);
		return -1;
	}		
	return numb;
	
}

void printMenu() 
{
	printf("Список доступных команд:\n");
	printf("help			| Выводит список доступных команд\n");
	printf("functions		| Доступных математических функций\n");
	printf("eval <expression>	| Считает значение выражения\n");
	printf("fact <number>		| Считает указанный факториал целого числа\n");
	printf("sqrt <number>		| Считает квадратный корень из указанного числа\n");
	printf("say <massage>		| Отправить сообщение серверу\n");
	printf("history			| Получить историю результатов запросов\n");	
	printf("%s			| Exit\n\n", EXIT);
}

void* rcvHandler(void *arg)
{
	int sock = (int) arg, flag = 0;;
	char msg[msg_length];
	
	for(;;) 
	{
		bzero(msg, msg_length);
		int res = readFix(sock, msg, msg_length, 0);		
		if (res <= 0) 
		{
			if (!res)
				printf("Server left.\n");
			else
				perror("read call failed: ");
			close(sock);
			pthread_exit(NULL);
			exit(1);
		}	
		
		switch(msg[0])
		{
			case 'e':
			case 'E':
			case 'f':
			case 'F':
			case 's':
			case 'S':
			{
				printf(CYN "Результат операции: " RESET);
				flag = 1;
				break;
			}
			case 'h':
			case 'H':
			{
				printf(CYN "История ваших запросов: " RESET "\n");
				flag = 1;
				break;
			}
			default: 
				break;
		}
		if (flag) 
		{	
			for (int i = 2; i < strlen(msg); i++)
			{
				printf( "%c" , msg[i]);
			}
			printf("\n");
		}
		else printf(CYN "%s" RESET "\n", msg);

	}
}

int strWithoutSpacesCmp(char* static_str, char* final_str) {
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
    // число байт в сообщении
    unsigned msgLength = strlen(buf);
    int res = send(sock, &msgLength, sizeof(unsigned), flags);
    if (res <= 0)
        return res;
	return send(sock, buf, msgLength, flags);
}
