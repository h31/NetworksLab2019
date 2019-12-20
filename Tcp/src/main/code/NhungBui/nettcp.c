#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h> 
#include <pthread.h>
#include "course.h"

void nettcp_bind_server(int *server, int port)
{
	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	*server = socket(AF_INET, SOCK_STREAM, 0);
	if (*server < 0) 
	{
		perror("Can't create socket to listen: ");
		exit(1);
	}

	if (setsockopt(*server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) 
	{
		perror("ERROR on setsockopt");
		shutdown(*server, how_shutdown);
		close(*server);
	}
	printf ("Enter <%s> to quit or </k <int>> to kill user with id <int>\n", EXIT);

	int res = bind(*server, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (res < 0) 
	{
		perror("Can't bind socket");
		shutdown(*server, how_shutdown);
		close(*server);
		exit(1);
	}
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

int sendFix(int sock, char* buf)
{
	// шлем число байт в сообщении
	unsigned msgLength = strlen(buf);
	int res = send(sock, &msgLength, sizeof(unsigned), 0);
	if (res <= 0)
		return res;
	res = send(sock, buf, msgLength, 0);
	if (res <= 0) 
	{
		perror("send call failed: ");
		close_client(sock);
	}
	return res;
}