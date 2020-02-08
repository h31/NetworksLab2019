#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <pthread.h>
#include <time.h>

struct Message {
	int length;
	char* message;
};

int mode = 0;

pthread_mutex_t mutex;

char* read_msg(int fd) {
	char tmp = '\n';
	int length = 2;
	int curr = 0;
	char * message = malloc(length * sizeof(char));
	do {
		read(fd, &tmp, 1);
		message[curr] = tmp;
		curr++;
		if (curr == length) {
			length *= 2;
			message = realloc(message, length * sizeof(char));
		}
	} while(tmp != '\n' && tmp != '\0');
	message = realloc(message, curr * sizeof(char));
	message[curr - 1] = '\0';
	printf("\033[1A\033[K");
    fflush(stdout);
	return message;
}

void print_with_time(char * message) {
  time_t rawtime;
  struct tm * timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  printf("<%d:%d>%s\n", timeinfo->tm_hour, timeinfo->tm_min, message);
}

char* msg_to(char* nickname, int nick_length, char* message, int message_length) {
	//4 cuz we need null c at the end;
	int length = (nick_length + message_length + 7);
	char* msg = malloc((length + 1) * sizeof(char));
	int al = length - 4;
    void* l = &al;
	char* len = (char*) l;
    for (int i = 0; i < sizeof(int); i++)
        *msg++ = len[i];
	*msg++ = '[';
	strcpy(msg, nickname);
	msg += nick_length;
	*msg++ = ']';
	*msg++ = ' ';
	strcpy(msg, message);
	msg -= (nick_length + 7);
	msg[length] = '\0';
	return msg;
}

struct Message * read_sock(int fd) {
	//read length on message
	struct Message* m = malloc(sizeof(struct Message));
	char bytes[4];
	read(fd, bytes, 4);
	int len;
	//char[4] -> int
	len = bytes[0] | ((int)bytes[1] << 8) | ((int)bytes[2] << 16) | ((int)bytes[3] << 24);
	//read message
	char message[len];
	m->length = len + 4;
	read(fd, message, len);
	char* msg = malloc((len + 4) * sizeof(char));
    for (int i = 0; i < len; i++)
        *msg++ = message[i];
    msg -= len;
	m->message = msg;
	return m;
}

void print_msg(struct Message * m) {
	print_with_time(m->message);
	free(m->message);
	free(m);
}

void* read_server(void* sock) {
    int socket = *(int *) sock;
    while (1)
        print_msg(read_sock(socket));
}
void disconnect(int socket, pthread_t *read_t) {
    pthread_cancel(read_t);
    shutdown(socket,SHUT_RDWR);
    close(socket);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    struct termios initial_settings, new_settings;
    pthread_t read_t;
    char key;

    int nick_length = strlen(argv[3]);
    char* nickname = malloc(nick_length * sizeof(char));
    strcpy(nickname, argv[3]);
    pthread_mutex_init(&mutex,NULL);
    tcgetattr(fileno(stdin), &initial_settings);

    if (argc < 4) {
        fprintf(stderr, "usage %s hostname port nickname\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR while opening socket");
        disconnect(sockfd, &read_t);
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
        disconnect(sockfd, read_t);
    }

    if (pthread_create(&read_t, NULL, read_server, &sockfd) < 0) {
        printf("ERROR");
        exit(1);
    }

    while (1) {
        if (!mode) {
            new_settings = initial_settings;
            new_settings.c_lflag &= ~ICANON;
            new_settings.c_lflag &= ~ECHO;
            new_settings.c_cc[VMIN] = 0;
            new_settings.c_cc[VTIME] = 0;
            tcsetattr(fileno(stdin), TCSANOW, &new_settings);
            read(0, &key, 1);

            if (key == 'm')
                mode = 1;

            if (key == 'q') {
                tcsetattr(fileno(stdin), TCSANOW, &initial_settings);
                disconnect(sockfd, &read_t);
            }
        }
        else {
            tcsetattr(fileno(stdin), TCSANOW, &initial_settings);

            char* message = read_msg(STDIN_FILENO);
            int message_length = strlen(message);
            char* to = msg_to(nickname, nick_length, message, message_length);
            free(message);
            print_with_time(to + 4);

            if (message_length > 1)
                write(sockfd, to, message_length + nick_length + 7);

            key = 0;
            mode = 0;
        }
    }

    return 0;

}
