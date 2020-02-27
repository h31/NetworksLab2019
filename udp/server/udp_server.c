#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "udp.h"

#define SERVER_PORT 6000
#define BUFFER_LEN  256

int main(int argc, char **argv)
{
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) printf("Socket creation failed\n");
	
	struct sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(SERVER_PORT);
	
	printf("Server socket: localhost:%d\n", SERVER_PORT);
	
	int bind_status = bind(
		s,
		(struct sockaddr *) &serv_addr,
		sizeof(serv_addr)
	);
	if (bind_status) printf("Bind socket failed\n");
	
	char buffer[BUFFER_LEN];
	bzero(buffer, BUFFER_LEN);
	
	while (1)
	{
		int status = custom_read(s, &serv_addr, sizeof(serv_addr), buffer, BUFFER_LEN);
		if (status == BUFFER_LEN)
		{
			printf("\033[1;36mClient message: [%s]\033[0m\n", buffer);
			if (!strcmp(buffer, "qq")) break;
		}
		else
		{
			printf("Error reading from socket - custom_read returned %d\n");
			break;
		}
	}
	
	if (close(s))
	{
		printf("Server socket closing failed\n");
	}
	
	printf("Server finished work\n");
	
	return 0;
}
