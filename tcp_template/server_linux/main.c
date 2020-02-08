#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>

#define max_clients 5
#define port 5001

pthread_mutex_t mutex;
int sockfd;

struct Message {
	int length;
	char* message;
};

int clients[max_clients];
struct pollfd clients_poll[max_clients];
int clients_count = 0;

void print_with_time(char * message) {
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	printf("<%d:%d>%s\n", timeinfo->tm_hour, timeinfo->tm_min, message);
}

void close_client(int socket) {
    shutdown(socket, SHUT_RDWR);
    close(socket);
    print_with_time("Client left us");

    for (int i = 0; i < max_clients; i++)
        if (clients[i] != NULL && clients[i] == socket)
            for (int j = i; j < clients_count; j++)
                clients[j] = clients[j + 1];

    for (int i = 1; i <= clients_count; i++) {
        if (clients_poll[i].fd == socket) {
            for (int j = i; j < clients_count; j++) {
                clients_poll[j] = clients_poll[j + 1];
            }
        }
    }
    clients_count--;
}

struct Message * read_sock(int fd) {
	//read length on message
	struct Message* m = malloc(sizeof(struct Message));
	char bytes[4];
	if (read(fd, bytes, 4) <= 0) {
        close_client(fd);
        return NULL;
	}
	int len;
	//char[4] -> int
	len = bytes[0] | ((int)bytes[1] << 8) | ((int)bytes[2] << 16) | ((int)bytes[3] << 24);
	//read message
	char message[len];
	m->length = len + 4;
	read(fd, message, len);
	char* msg = malloc((len + 4) * sizeof(char));
	for (int i = 0; i < 4; i++)
		*msg++ = bytes[i];
	for (int i = 0; i < len; i++)
		*msg++ = message[i];
	msg -= (len + 4);
	m->message = msg;
	return m;
}

void propagate_message(struct Message * message, int socket){
    if (message == NULL)
        return;
	for (int i = 0; i < max_clients; i++)
		if (clients[i] != NULL)
			if (clients[i] != socket)
				if (write(clients[i], message->message, message->length) <= 0) {
					close_client(clients[i]);
					clients[i] = NULL;
				}
}

void server_exit() {
    for (int i = 0; i < clients_count; i++)
        close_client(clients[i]);
    close(sockfd);
    exit(1);
}

int main(int argc, char *argv[]) {
	int newsockfd;
	uint16_t portno;
	unsigned int clilen;
	struct sockaddr_in serv_addr, cli_addr;

	signal(SIGINT, server_exit);
	pthread_mutex_init(&mutex,NULL);
	bzero(&clients, sizeof(int) * max_clients);
	pthread_t clientTid;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0)
		perror("ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = port;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		exit(1);
	}

	listen(sockfd, max_clients);
	clilen = sizeof(cli_addr);
    clients_poll[clients_count].fd = sockfd;
    clients_poll[clients_count].events = POLLIN;
    clients_count++;

	while (1){
	    int p = poll(clients_poll, (unsigned int) max_clients, 5000);

        for (int i = 0; i < clients_count; i++) {
            if (clients_poll[i].revents == 0) {
                continue;
            }
            if (clients_poll[i].fd == sockfd) {
                newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                clients_poll[clients_count].fd = newsockfd;
                clients_poll[clients_count].events = POLLIN;
                if (newsockfd < 0) {
                    perror("ERROR on accept");
                    exit(1);
                } else {
                    print_with_time("New client connected");

                    for (int i = 0; i <= clients_count; i++) {
                        if (clients[i] == NULL){
                            clients[i] = newsockfd;
                            break;
                        }
                    }
                    clients_count++;
                }
            } else {
                propagate_message(read_sock(clients_poll[i].fd), clients_poll[i].fd);
            }

        }

	}

	return 0;
}
