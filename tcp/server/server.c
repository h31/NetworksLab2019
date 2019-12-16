#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Client
{
    int sock;
    char* name;
    struct Client *nextClient
} clientsList;

clientsList *first, *last;

int nClients = 0;

char* get_time();

void* getName(void* args);

void client_processing();

void remove_newline_ch(char *line);

int readMessage(int fromSock, char* buffer);

int readn(int fildes, char* ptr, size_t n);

int sendAll(clientsList *client, char* name, char* buffer);

int sendToServer(int destination, char* content);

clientsList *addClient(int socket);

clientsList *addClientInfo(int socket);

void disconnect(clientsList *client);

int main(int argc, char *argv[]) {
	int sockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);


    int newsockfd;
    int client_id;

    while(1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) {
            perror("ERROR on accept");
            break;
        }

        clientsList *client = addClient(newsockfd);

        pthread_t tid;
         if (pthread_create(&tid, NULL, getName, (void *) client) != 0) {
            printf("thread has not created");
            exit(1);
        }


    }
    return 0;
}


void* getName(void* args){
    clientsList *iterrClient = first;
	int individual_name = 0;
	clientsList *client = (clientsList *) args;
    int client_socket = client->sock;
	char user_name[20];
	char* msg[300] = {0};
	//sprintf(msg, "Enter your name down there:");
	//ssendIndividual(client, msg);
	while ( individual_name == 0 ) {
		readMessage(client_socket, user_name);
		remove_newline_ch(user_name);
		if (strcmp(user_name, "") == 0)
    		{
        		sprintf(msg, "Such name already exists. Choose another name:");
			sendIndividual(client, msg);
			continue;
    		}
		if ( nClients == 1 ) {
			individual_name = 1;
		}
		for (int i = 0; i < nClients; ++i)
    		{   
			if (iterrClient == client)
        		{
           			continue;
        		}
		
        		if (strcmp(iterrClient->name, user_name) != 0) {
            			individual_name = 1;
       			}
			else {
				    individual_name = 0;
    				sprintf(msg, "Such name already exists. Choose another name:");
				    sendIndividual(client, msg);
				break;
			}
    		}
		
	}
	pthread_mutex_lock(&mutex);
    //client->name = (char *) malloc(sizeof(user_name))    
    client->name = user_name;
    pthread_mutex_unlock(&mutex);
	
	client_processing(client, client_socket, user_name);
}

int readn(int fildes, char* ptr, size_t n){
    int result = 0;
    int readedBytes = 0;
    while(result < n){
        readedBytes = read(fildes, ptr + result, n);
        if (readedBytes < 0){
	    perror("ERROR reading from socket");
            exit(1);
        }
        result += readedBytes;
    }
    return result;
}

void client_processing(clientsList *client, int client_socket, char* user_name){
	char buffer[256];

    	while (1) {
        if (readMessage(client_socket, buffer) <= 0){
            break;
        }
        remove_newline_ch(buffer);
        printf("RIGHT NOW BUFFER IS %s and its length is %d \n", buffer , strlen(buffer));
        if (strcmp(buffer, "/stop") == 0)
        {
            printf("STRING ARE THE SAME");
            disconnect(client);  //TODO disconnect function
        }
        else 
        {
            sendAll(client, user_name, buffer);
            printf("\n");
        }
        fflush(stdout);
    }
}

int readMessage(int fromSock, char* buffer){
	    bzero(buffer, sizeof(buffer));
	    int tmp;
	    int size;
	    if (readn(fromSock, &size, sizeof(int)) < 0) {
		perror("ERROR reading from socket: ");
		exit(1);
	    }

	    tmp = readn(fromSock, buffer, size);
	    if (tmp < 0) {
		perror("ERROR reading from socket: ");
		exit(1);
	    }
        printf("SIZE OF READED MESSAGE IS %d\n", size);
	    return tmp;
}

int sendAll(clientsList *client, char* name, char* buffer){
    clientsList *newClient = first;
    char* formedMessage[300];
    bzero(formedMessage, sizeof(formedMessage));
    sprintf(formedMessage, "[%s] <%s> : %s",get_time() ,name, buffer);
    printf("SENDING MESSAGE TO ALL: \n%s\n", formedMessage);
    for (int i = 0; i < nClients; ++i)
    {   
        printf("iterration number %d\n", i);
        printf("NAMES ARE %s, AND %s \n", newClient->name, client->name);
        printf("CLIENTS IS : %s AND SOCKET IS %d \n", newClient->name, newClient->sock); //newClient->nextClient->name ); 
        if (strcmp(newClient->name, client->name) == 0)
        {
            printf("I Identify myself\n");
            newClient = newClient->nextClient;
            continue;
        }
        sendToServer(newClient->sock, formedMessage);
        newClient = newClient->nextClient;
        printf("Message sent\n");
    }
}

int sendIndividual(clientsList *client, char* buffer){
	char* formedMessage[300] = {0};
    sprintf(formedMessage, " <SERVER> : %s \n", buffer);
	sendToServer(client->sock, formedMessage);
}

int sendToServer(int destination, char* content){
    int check;
    int size = strlen(content) + 1;
    check = write(destination, &size, sizeof(int));
    if (check <= 0)
    {
        printf("Sending error\n");
    }
    check = write(destination, content, size);
    if (check <= 0)
    {
        printf("Sending error\n");
    }
}

char *get_time() {
    int hours, mintues, seconds, day, month, year;
    char *str;
    char tmp[6];
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    bzero(tmp, 6);
    strftime(tmp, 6, "%H:%M", local);
    str = (char *) malloc(sizeof(tmp));
    strcpy(str, tmp);
    return str;
}

void remove_newline_ch(char *line)
{
    int new_line = strlen(line) - 1;
    if (line[new_line] == '\n') {

        line[new_line] = '\0';
    }
}

clientsList *addClient(int socket) {
    if (nClients == 0) {
        first = addClientInfo(socket);
        last = first;
        nClients++;
        return first;
    }
    clientsList *client = addClientInfo(socket);
    last->nextClient = client;
    last = client;
    nClients++;
    return client;
}

clientsList *addClientInfo(int socket) {
    clientsList *client = (clientsList *) malloc(sizeof(clientsList));
    client->sock = socket;
    client->name = NULL;
    client->nextClient = NULL;
    return client;
}

void disconnect(clientsList *client) {
    clientsList *iterrClient = first;
    close(client->sock);
    if (nClients == 1 && first == client) {
        first = NULL;
        last = NULL;
    } else if (nClients != 1 && first == client)
    {
        first = client->nextClient;
    } else {
        while (strcmp(iterrClient->nextClient->name, client->name) != 0)
        {
            printf("ITERRATING CLIENTS\n");
            iterrClient = iterrClient->nextClient;
        }
        if (client->nextClient == NULL) {
            last = iterrClient;
            iterrClient->nextClient = NULL;
        } else {
            iterrClient->nextClient = client->nextClient;
        } 
    }
    nClients--;
    free(client);
    pthread_exit(NULL);
}

