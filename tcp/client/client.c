
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"
#define name_length 40
#define msg_length 256
#define total_length (name_length + msg_length + sizeof("<>:\r\n"))
#define EXIT "/exit"
#define msg_mode "m"
#define cmd_length 10

int en_msg = 0;
char* nickname;
pthread_mutex_t mp;

void* sendHandler(void*);
void* rcvHandler(void*);
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
        printf("Illegal amount of aguments. Enter your nickname\n");
        exit(1);
    }
    else if (strlen(argv[1]) > name_length) {
        printf("Length of your nick name must be less than %d\n", name_length);
        exit(1);
    }
    else
        nickname = argv[1];

    if (argc < 3) {
        printf("Using default addr %s\n", DEF_IP);
        addr = DEF_IP;
    }
    else
        addr = argv[2];
    if (argc < 4) {
        printf("Using default port %d\n", DEF_PORT);
        port = DEF_PORT;
    }
    else
        port = atoi(argv[3]);

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
	int sock = (int) arg;
	char msg[msg_length];
	char text[total_length];
	char cmd[cmd_length];

	printf("Enter <%s> to write your message\n", msg_mode);
	for(;;) 
	{
		bzero(cmd, cmd_length);
		bzero(msg, msg_length);
		bzero(text, total_length);
		
		fgets(cmd, cmd_length, stdin);
		cmd[strlen(cmd) - 1] = '\0';
		
		if (!strCmp(cmd, msg_mode)) {
		    //pthread_mutex_lock(&mp);
			en_msg = 1;
			//pthread_mutex_unlock(&mp);

			fgets(msg, msg_length, stdin);
			msg[strlen(msg) - 1] = '\0';

			if (!strCmp(msg, EXIT)) {
				printf("Bye-bye\n");
				close(sock);
				exit(1);			
			}
	
			strcpy(text, "[");
			strcat(text, nickname);
			strcat(text, "] ");
			strcat(text, msg);
			
			int res = sendFix(sock, text, 0);
			if (res <= 0) {
				perror("send call failed");
				close(sock);
				pthread_exit(NULL);
				exit(1);
			}	
		}
		else printf("Illegal input\n");
		
		//pthread_mutex_lock(&mp);
		en_msg = 0;
		//pthread_mutex_unlock(&mp);
	}
	
}

void* rcvHandler(void *arg)
{
	int sock = (int) arg;
	char msg[msg_length];
	
	for(;;) {
		//pthread_mutex_lock(&mp);
		if(!en_msg){
			bzero(msg, msg_length);
			int res = readFix(sock, msg, msg_length, 0);		
			if (res <= 0) {
				perror("send call failed");
				close(sock);
				pthread_exit(NULL);
				exit(1);
			}	
			
			if(!strCmp(msg, "/goodbye")) {
				printf("Server left\n");
				close(sock);
				exit(1);
			} 
            while (en_msg)
    			usleep(1000);
			printf("%s\n", msg);
		}
		//pthread_mutex_unlock(&mp);
	}
}

int strCmp(char* static_str, char* final_str) {
	int count = 0;
	char noblank_str[total_length];
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
    send(sock, buf, msgLength, flags);
}
