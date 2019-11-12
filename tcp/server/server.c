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

#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"
#define how_shutdown 2 //зарпещена передача данных
#define msg_length 256
#define time_length 50
#define name_length 40
#define total_length (msg_length + time_length + name_length + sizeof("<>:\r\n"))
#define max_clients 2
#define EXIT "/exit"

int clients_sockets[max_clients];
pthread_t clients_threads[max_clients];
pthread_mutex_t mp;

int readN(int sock, char* buf, int length, int flags);
int readFix(int sock, char* buf, int bufSize, int flags);
int sendFix(int sock, char* buf, int flags);

// обработка одного клиента
void* clientHandler(void* arg)
{
	int pos = (int) arg;
    int sock = clients_sockets[pos];
    char msg_buf[msg_length];
    char time_buf[time_length];
	char text[total_length];
    int res = 0;
    time_t curtime;
    struct tm* loc_time;

    for (;;) {
        bzero(msg_buf, msg_length);
		bzero(time_buf, time_length);
		bzero(text, total_length);

        res = readFix(sock, msg_buf, msg_length, 0);
        if (res <= 0) {
            perror("A client left chat:");

			pthread_mutex_lock(&mp);
			clients_sockets[pos] = 0;
			pthread_mutex_unlock(&mp);
            
			shutdown(sock, how_shutdown);
            close(sock);
            pthread_exit(NULL);
        }

        curtime = time(NULL);
        loc_time = localtime(&curtime);
        strftime(time_buf, time_length, "<%I:%M %p> ", loc_time);
	
		strcpy(text, time_buf);
        strcat(text, msg_buf);

        printf("%s\n", text);
		
		pthread_mutex_lock(&mp);
		for(int i = 0; i < max_clients; i++) {
			if (clients_sockets[i] != 0) {
				res = sendFix(clients_sockets[i], text, 0);
				if (res <= 0) {
					perror("send call failed");
					shutdown(sock, how_shutdown);
					close(sock);
					pthread_exit(NULL);
				}
			}
        }
        pthread_mutex_unlock(&mp);

    }
}

void* acceptHandler(void* arg)
{
	int server = (int) arg;
    struct sockaddr_in cli_addr;
	unsigned int clilen = sizeof(cli_addr);
	int cli_sock;
	int res;
	for (;;) {
		int free_pos = -1;
		pthread_mutex_lock(&mp);
        for (int i = 0; i < max_clients; i++) {
			if (clients_sockets[i] == 0) {
				free_pos = i;
				break;
			}
		}
		pthread_mutex_unlock(&mp);
		
		if (free_pos == -1) {
			continue;
		}
		
		cli_sock = accept(server, (struct sockaddr*) &cli_addr, &clilen);
		if (cli_sock < 0) {
			perror("ERROR on accept");
			exit(1);
		}
		
		pthread_mutex_lock(&mp);
		clients_sockets[free_pos] = cli_sock;
		pthread_mutex_unlock(&mp);	
		
        puts("\nNew client connected!\n");
       
        res = pthread_create(&clients_threads[free_pos], NULL, clientHandler, (void*)(free_pos));
        if (res) {
            printf("Error while creating new thread\n");
        }
		
    }

}

int main(int argc, char** argv)
{
    
    for (int i = 0; i < max_clients; i++) {
        clients_sockets[i] = 0;
    }

    int port = 0;
    if (argc < 2) {
        printf("Using default port %d\n", DEF_PORT);
        port = DEF_PORT;
    }
    else
        port = atoi(argv[1]);

	printf ("Enter <%s> to quit\n", EXIT);
	
	if (pthread_mutex_init(&mp, NULL) != 0) {
		perror("ERROR on mutex:");
		exit(1);
	}
	
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("Can't create socket to listen: ");
        exit(1);
    }

    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        perror("ERROR on setsockopt");
        shutdown(server, how_shutdown);
        close(server);
    }

    int res = bind(server, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (res < 0) {
        perror("Can't bind socket");
        shutdown(server, how_shutdown);
        close(server);
        exit(1);
    }
    // слушаем входящие соединения
    res = listen(server, max_clients);
    if (res) {
        perror("Error while listening:");
        exit(1);
    }
    puts("Waiting for incoming connections");
    // основной цикл работы
	
	pthread_t tid;
	res = pthread_create(&tid, NULL, acceptHandler, (void*) server);
	
	for (;;) {
		char text[msg_length];
		fgets(text, msg_length, stdin);
		text[strlen(text) - 1] = '\0';
		
		if (!strCmp(text, EXIT)) {	
			printf("Goodbye!\n");
			pthread_cancel(tid);
			strcpy(text, "/goodbye");
			
	        pthread_mutex_lock(&mp);
			for (int i = 0; i < max_clients; i++) {
				if (clients_sockets[i] != 0){
					sendFix(clients_sockets[i], text, 0);
					close(clients_sockets[i]);
					pthread_cancel(clients_threads[i]);
				}
			}
	        pthread_mutex_unlock(&mp);

			close(server);
			exit(1);
		}
		printf ("Illegal input\n");
	}
	
    shutdown(server, how_shutdown);
    close(server);
    return 0;
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
    // шлем число байт в сообщении
    unsigned msgLength = strlen(buf);
    int res = send(sock, &msgLength, sizeof(unsigned), flags);
    if (res <= 0)
        return res;
    send(sock, buf, msgLength, flags);
}
