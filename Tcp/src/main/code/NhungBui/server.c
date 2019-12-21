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

void init()
{
	for (int i = 0; i < max_clients; i++) 
	{
		clients_sockets[i] = 0;
	}
	
	if (pthread_mutex_init(&mp, NULL) != 0) 
	{
		perror("ERROR on mutex:");
		exit(1);
	}
}

int main(int argc, char** argv)
{   
	init();
	int port = 0;
	if (argc < 2)
	{
		printf("Using default port %d\n", DEF_PORT);
		port = DEF_PORT;
	}
	else
		port = atoi(argv[1]);
	
	int server;
	nettcp_bind_server(&server, port);
	// слушаем входящие соединения
	int res = listen(server, max_clients);
	if (res) 
	{
		perror("Error while listening:");
		exit(1);
	}
	puts("Waiting for incoming connections");
	// основной цикл работы
	
	pthread_t tid;
	res = pthread_create(&tid, NULL, acceptHandler, (void*) server);
	
	killClient(tid, server);
	shutdown(server, how_shutdown);
	close(server);
	return 0;
}