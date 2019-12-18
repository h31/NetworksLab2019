#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <bits/fcntl-linux.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>

#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"
#define how_shutdown 2 //зарпещена передача данных
#define msg_length 256
#define time_length 50
#define name_length 40
#define total_length (msg_length + time_length + name_length + sizeof("<>:\r\n"))
#define max_clients 50
#define max_fds (max_clients + 1)	//clients + server
#define EXIT "/exit"

void communicate_with_client (int fd);
void close_server ();
void close_client (int cli_sock);
int strCmp(char* static_str, char* final_str);
int readN(int sock, char* buf, int length, int flags);
int readFix(int sock, char* buf, int bufSize, int flags);
int sendFix(int sock, char* buf, int flags);
void printFds();

struct pollfd fds[max_fds];
int poll_size, server;

int main(int argc, char** argv)
{
	int port = 0;
	if (argc < 2) 
	{
		printf("Using default port %d\n", DEF_PORT);
		port = DEF_PORT;
	}
	else
		port = atoi(argv[1]);
	
	if (signal(SIGINT, close_server) == SIG_ERR) {
		perror("ERROR on sigint_handler");
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
		exit(1);
	}
	
	if (fcntl(server, F_SETFL, O_NONBLOCK) < 0) 
	{
		perror("ERROR on making socket nonblock:");
		close(server);
		exit(1);
	}
	
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
	
	//initiate pollfd	
	fds[0].fd = server;
	fds[0].events = POLLIN;
	poll_size = 1;
	
	for (int i = 1; i < max_fds; i++) 
			fds[i].fd = -1;
	
	struct sockaddr_in cli_addr;
	unsigned int cli_len = sizeof(cli_addr);
	
	for (;;) 
	{
		switch (poll(fds, poll_size, -1)) 
		{
			case 0: //time out
				printf("server timeout\n");
				break;
			case -1: //error
				perror("ERROR on poll");
				break;
			default:
			{
				for (int i = 0; i < poll_size; i++) 
				{ 
					if (fds[i].revents ==0) 
						continue;
					
					//accept client's connect
					if((fds[i].fd == server) && (fds[i].revents & POLLIN))  
					{
						for(;;) 
						{
							int cli_sock = accept(server, (struct sockaddr*) &cli_addr, &cli_len);
							if (cli_sock < 0) 
							{
								if (errno != EWOULDBLOCK)
								{
									perror("  accept() failed");
									close_server();
								}
								break;
							}
	
							poll_size++;
							fds[poll_size - 1].fd = cli_sock;
							fds[poll_size - 1].events = POLLIN;
							
							puts("\nNew client connected!\n");
						}
					}
					else 
					{
						switch (poll(&fds[i], 1, -1)) 
						{
							case 0: //time out
								printf("server timeout\n");
								break;
							case -1: //error
								perror("ERROR on poll");
								break;
							default:
							{
								if (fds[i].revents & POLLIN) 
									communicate_with_client (fds[i].fd);		
							}
						}
					}				
				}
			}
		}
	}
	
	close_server();
	return 0;
}

void communicate_with_client (int fd) {
	char msg_buf[msg_length];
	char time_buf[time_length];
	char text[total_length];
	time_t curtime;
	struct tm* loc_time;
	
	bzero(msg_buf, msg_length);
	bzero(time_buf, time_length);
	bzero(text, total_length);
	
	int res = readFix(fd, msg_buf, msg_length, 0);
	
	if (res <= 0) 
	{
		if (errno != EWOULDBLOCK)
		{
			perror("Server: ERROR on recv():");
			close_client(fd);
			return;
		}
		else 
		{
			close_client(fd);
			printf("A client left chat!\n");
		}
	}
	else 
	{	
		curtime = time(NULL);
		loc_time = localtime(&curtime);
		strftime(time_buf, time_length, "<%I:%M %p> ", loc_time);
		strcpy(text, time_buf);
		strcat(text, msg_buf);
		
		printf("%s\n", text);
			
		for(int j = 1; j < poll_size; j++) 
		{
			res = sendFix(fds[j].fd, text, 0);
			if (res < 0) 
			{
				perror("Server: send call failed");
				close_client(fds[j].fd);
			}
		}
	}
}

void close_server () 
{
	for (int i = 1; i < poll_size; i++) 
	{
		if (fds[i].fd != -1) close_client(fds[i].fd);
	}
	close(server);
	exit(1);
}

void close_client (int cli_sock) 
{
	close(cli_sock);
	for (int i = 1; i < poll_size; i++) 
	{
		if (fds[i].fd == cli_sock) 
		{
			if (i != poll_size - 1) 
				fds[i].fd = fds[poll_size - 1].fd; 
			else
				fds[i].fd  = -1;
			break;
		}
	}
	poll_size--;
}	

int strCmp(char* static_str, char* final_str) 
{
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

int readN(int sock, char* buf, int length, int flags) 
{
	int left;
	int read;
	char* ptr;
	
	ptr = buf;
	left = length;
	while (left > 0) 
	{
		if ((read = recv(sock, ptr, left, flags)) <= 0) 
		{
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