#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define max_clients 5
#define port 5002

pthread_mutex_t mutex;
int sockfd;

struct Message {
	int length;
	char* message;
};

int clients[max_clients];
int clients_count = 0;

void print_with_time(char * message) {
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	printf("<%d:%d>%s\n", timeinfo->tm_hour, timeinfo->tm_min, message);
}

void close_client(int socket) {
	pthread_mutex_lock(&mutex);
	close(socket);
	for (int i = 0; i < max_clients; i++) {
		if(clients[i] != NULL && clients[i] == socket) {
			print_with_time("Client left us");
			clients[i] = NULL;
			break;
		}
	}
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}

struct Message * read_sock(int fd) {
	//read length on message
	struct Message* m = malloc(sizeof(struct Message));
	char bytes[4];
	if (read(fd, bytes, 4) <= 0)
		close_client(fd);
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
	pthread_mutex_lock(&mutex);
	for (int i = 0; i < max_clients; i++)
		if (clients[i] != NULL)
			if (clients[i] != socket)
				if (write(clients[i], message->message, message->length) <= 0) {
					close_client(clients[i]);
					clients[i] = NULL;
				}
	pthread_mutex_unlock(&mutex);

}

void* client_thread(void * c) {
	int client = *(int*) c;
	while (1)
		propagate_message(read_sock(client), client);
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

	while (1){
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if (newsockfd < 0) {
			perror("ERROR on accept");
			exit(1);
		} else {
			pthread_mutex_lock(&mutex);
			print_with_time("New client connected");
			int client_number = 0;

			for (int i = 0; i <= clients_count; i++) {
				if (clients[i] == NULL){
					clients[i] = newsockfd;
					client_number = i;
					break;
				}
			}

			pthread_mutex_unlock(&mutex);
			pthread_create(&clientTid, NULL, client_thread, &clients[client_number]);
			clients_count++;
		}
	}

	return 0;
}
