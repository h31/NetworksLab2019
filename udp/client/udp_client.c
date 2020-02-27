
/*
 * 
 * Клиент UDP
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 6000
#define BUFFER_LEN  256

#include "udp.h"

int main(int argc, char **argv)
{
	int c = socket(AF_INET, SOCK_DGRAM, 0);
	if (c < 0) printf("Socket creation failed\n");
	
	struct sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
	serv_addr.sin_port = htons(SERVER_PORT);
	printf("Server socket: %s:%d\n", SERVER_ADDR, SERVER_PORT);
	
	char buffer[BUFFER_LEN];
	bzero(buffer, BUFFER_LEN);
	
	int read_status = 1;
	
	while (read_status > 0 && strcmp(buffer, "q") && strcmp(buffer, "qq"))
	{
		bzero(buffer, BUFFER_LEN);
		fgets(buffer, BUFFER_LEN, stdin);
		buffer[strlen(buffer) - 1] = '\0'; // Remove \n in the end
		
		int write_status = custom_write(c, &serv_addr, sizeof(serv_addr), buffer, BUFFER_LEN);
		if (write_status != BUFFER_LEN) printf("Error writing data\n");
		
		printf("\033[1;32m[%s]\033[0m\n", buffer);
	}
	
	if (close(c)) printf("Socket closing failed\n");
	
	return 0;
}
