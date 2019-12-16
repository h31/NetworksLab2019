#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>


int sendToServer(int destination, char* content);

void* recieveMessage(void* args);

int readMessage(int fromSock, char* buffer);

void* sendMessage(void* args);

int readn(int fildes, char* ptr, size_t n);

void remove_newline_ch(char *str);

int main(int argc, char *argv[]) {

    int sockfd;
    int n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    printf("Create a socket point\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    pthread_t tid_handleMSG;
    if (pthread_create(&tid_handleMSG, NULL, recieveMessage, &sockfd) != 0) {
        printf("thread has not created");
        exit(1);
    }
    if (argc == 3) {
        printf("Enter your name down there:");
    } else {
        sendToServer(sockfd, argv[3]);
    }
    
    pthread_t tid_writeMSG;
    if (pthread_create(&tid_writeMSG, NULL, sendMessage, &sockfd) != 0) {
        printf("thread has not created");
        exit(1);
    }
    
    pthread_join(tid_handleMSG, NULL); //waiting for threads finish
    pthread_join(tid_writeMSG, NULL);
    return EXIT_SUCCESS;

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

void* recieveMessage(void* args){

    int readSocket = *(int *) args;
    char* readBuffer[300] = {0};
    while (1) {
        if (readMessage(readSocket, readBuffer) <= 0){
            break;
        }
        printf("\33[2K\r");
        printf(readBuffer);
        printf("\n");
        printf("Your message: "); 
        fflush(stdout);
        //fflush(stdin);
    }
}


int readMessage(int fromSock, char* buffer){
	    bzero(buffer, sizeof(buffer));
	    int tmp;
	    int size;
	    if (readn(fromSock, &size, sizeof(int)) < 0) {
		perror("ERROR reading from socket");
		exit(1);
	    }

	    tmp = readn(fromSock, buffer, size);
	    if (tmp < 0) {
		perror("ERROR reading from socket");
		exit(1);
	    }
	    return tmp;
}

void* sendMessage(void* args){

    char* writeBuffer[300] = {0};
    int writeSocket = *(int *) args;
    
    while (1) {
        printf("Your message: ");
	    bzero(writeBuffer, sizeof(writeBuffer));
    	fgets(writeBuffer, sizeof(writeBuffer) - 1, stdin);     
        //remove_newline_ch(writeBuffer);
    	sendToServer(writeSocket, writeBuffer);
    }
}

int sendToServer(int destination, char* content){
    int check;
    int size = strlen(content) + 1;
    check = write(destination, &size, sizeof(int));
    if (check <= 0)
    {
        printf("Sedning error\n");
    }
    check = write(destination, content, size);
    if (check <= 0)
    {
        printf("Sedning error\n");
    }
}

void remove_newline_ch(char *line)
{
    int new_line = strlen(line) - 1;
    if (line[new_line] == '\n') {
        line[new_line] = '\0';
    }
}
