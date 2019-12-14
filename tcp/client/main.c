#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <termios.h>

#include "protocol.h"


#define CACHE_SIZE 100
#define CACHE_NORMAL 0
#define CACHE_OVERFLOW 1

#define CMD_EXIT "!exit"
#define CMD_MODE_SWAP 'm'

#define MODE_LISTEN 0
#define MODE_WRITE 1

#define EXIT_OK 0
#define EXIT_ER 1


void init_mode();
void set_mode(int new_mode);
void login(int sockfd);
void exit_chat(int sockfd, int exit_code);
void output_handler(char* buffer);
void print_cached_msg();
void* th_msg_read_handler(void* args);
void* th_msg_send_handler(void* args);
void sig_close_handler(int sig);

struct Cache
{
    char *data[CACHE_SIZE];
    int counter;
    int flag;
} cached_msg;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int sockfd;
int mode;
static struct termios settings_write;
static struct termios settings_listen;


int main(int argc, char *argv[]) {
    int n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[MAX_MSG_LEN];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(EXIT_OK);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(EXIT_ER);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(EXIT_OK);
    }    

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    signal(SIGINT, sig_close_handler);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(EXIT_ER);
    }

    // init client mode (key pressed handler)
    init_mode(); 
    set_mode(MODE_WRITE);
    cached_msg.flag = CACHE_NORMAL;

    login(sockfd);    

    pthread_t tid_read;
    if (pthread_create(&tid_read, NULL, th_msg_read_handler, NULL) < 0) {
        printf("ERROR on thread create");
        exit(EXIT_ER);
    }

    pthread_t tid_send;
    if (pthread_create(&tid_send, NULL, th_msg_send_handler, NULL) < 0) {
        printf("ERROR on thread create");
        exit(EXIT_ER);
    }
    
    pthread_join(tid_read, NULL);
    pthread_join(tid_send, NULL);
    set_mode(MODE_WRITE);
    return EXIT_OK;
}

void init_mode()
{ 
    tcgetattr(0, &settings_write);
 
    settings_listen = settings_write;
 
    settings_listen.c_lflag &= (~ICANON);
    settings_listen.c_lflag &= (~ECHO);
    settings_listen.c_cc[VTIME] = 0;
    settings_listen.c_cc[VMIN] = 1;
}

void set_mode(int new_mode)
{
    if (new_mode == MODE_WRITE)
    {
        tcsetattr(0, TCSANOW, &settings_write);
    }
    else if (new_mode == MODE_LISTEN)
    {
        tcsetattr(0, TCSANOW, &settings_listen);
    }
    else
    {
        printf("\nInvalid client mode.\n");
        return;
    }    
    mode = new_mode;
}

void login(int sockfd)
{
    char nickname[MAX_NAME_LEN];
    printf("- Enter your name (%d-%d characters): ", MIN_NAME_LEN, MAX_NAME_LEN);
    bzero(nickname, sizeof(nickname)); 
    scanf("%s", nickname);
    int name_len = strlen(nickname);
    if (name_len < MIN_NAME_LEN || name_len > MAX_NAME_LEN)
    {
        printf("\n- Your name is invalid. Try another one.\n");
        login(sockfd);
    }
    else
    {
        send_msg(sockfd, nickname);
        bzero(nickname, sizeof(nickname)); 
        printf("- Now you're able to use chat.\n");
    }    
}

void exit_chat(int sockfd, int exit_code)
{
    printf("\nLeaving chat..\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    printf("Connection closed\n");
    set_mode(MODE_WRITE);
    exit(exit_code);
}

void output_handler(char* buffer)
{
    if (mode == MODE_LISTEN)
    {
        printf("%s\n", buffer);
    }
    else
    {
        pthread_mutex_lock(&mutex);
        if (cached_msg.counter < CACHE_SIZE)
        {
            cached_msg.data[cached_msg.counter] = strdup(buffer);
            cached_msg.counter++;
        }
        else
        {
            // loop msg save order
            if (cached_msg.flag == CACHE_NORMAL)
            {
                cached_msg.flag = CACHE_OVERFLOW;
                char* lost_msg = "- Few missed messages..";
                cached_msg.data[0] = strdup(lost_msg);
            }                      
            cached_msg.data[1] = strdup(buffer);
            cached_msg.counter = 2;
        }        
        pthread_mutex_unlock(&mutex);
    }    
}

void print_cached_msg()
{
    int count;
    pthread_mutex_lock(&mutex);
    if (cached_msg.flag == CACHE_OVERFLOW)
    {
        count = cached_msg.counter;
        while (count < CACHE_SIZE)
        {
            printf("%s\n", cached_msg.data[count]);
            free(cached_msg.data[count]);
            count++;
        }
    }
    count = 0;
    while (count < cached_msg.counter)
    {
        printf("%s\n", cached_msg.data[count]);
        free(cached_msg.data[count]);
        count++;
    }
    // cache reset
    cached_msg.counter = 0;
    cached_msg.flag = CACHE_NORMAL;
    pthread_mutex_unlock(&mutex);
}

void* th_msg_read_handler(void* args)
{
    char buffer[FULL_MSG_LEN];
    while (read_msg(sockfd, buffer) > 0)
    {
        output_handler(buffer);
        fflush(stdout);
        bzero(buffer, sizeof(buffer));
    }
}

void* th_msg_send_handler(void* args)
{
    char buffer[MAX_MSG_LEN];
    
    while (1)
    {         
        set_mode(MODE_LISTEN);

        char ch;
        while ((ch = getc(stdin)) != CMD_MODE_SWAP)
        {
            printf("- You are in a LISTEN mode. Press 'm' to enter WRITTING mode\n");
        }

        set_mode(MODE_WRITE);
        printf("\n=================== YOUR MESSAGE ===================\n");    
        bzero(buffer, sizeof(buffer));        
        fgets(buffer, sizeof(buffer) - 1, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // del newline ending
        printf("======================= END ========================\n\n");

        print_cached_msg();
        
        if (strcmp(buffer, "\0") == 0)
        {
            continue;
        }
        else if (strcmp(buffer, CMD_EXIT) == 0)
        {
            exit_chat(sockfd, EXIT_OK);
        }
        else
        {
            send_msg(sockfd, buffer); 
        }      
    }
}

void sig_close_handler(int sig)
{
    if (sig != SIGINT) return;
    exit_chat(sockfd, EXIT_ER);
}
