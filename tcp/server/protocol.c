#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>


#define TIME_MSG_LEN 20
#define BASE_YEAR 1900


int read_n(int sockfd, char* buffer, int length)
{
    int result = 0;
    int rd_bytes = 0;
    int msg_size = length;
    while(msg_size > 0)
    {
        rd_bytes = read(sockfd, buffer + result, msg_size);
        if (rd_bytes <= 0)
        {
            return -1;
        }
        result += rd_bytes;
        msg_size -= rd_bytes;
    }
    return result;
}

int read_msg(int sock, char* buffer)
{
    bzero(buffer, sizeof(buffer));
    int check;
    int size;
    check = read_n(sock, &size, sizeof(int));
    if (check < 0)
    {
        if (errno != EWOULDBLOCK)
        {
            perror("ERROR reading from socket");
        }
        return check;
    }
    check = read_n(sock, buffer, size);
    if (check < 0)
    {
        if (errno != EWOULDBLOCK)
        {
            perror("ERROR reading from socket");
        }
        return check;
    }
    return check;
}

int send_msg(int sock, char* buffer)
{
    int check;
    int size = strlen(buffer);
    check = write(sock, &size, sizeof(int));
    if (check <= 0)
    {
        if (errno != EWOULDBLOCK)
        {
            perror("ERROR sending to socket");
        }
        return check;
    }
    check = write(sock, buffer, size);
    if (check <= 0)
    {
        if (errno != EWOULDBLOCK)
        {
            perror("ERROR sending to socket");
        }
        return check;
    }
}

void get_time(char* time_holder)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(time_holder,
            "%d-%d-%d %d:%d:%d",
            tm.tm_year + BASE_YEAR,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec);
}

void build_msg(char* full_msg, char* name, char* buffer)
{
    char* time_msg[TIME_MSG_LEN] = {0};
    get_time(time_msg);
    sprintf(full_msg, "<%s> [%s] : %s", time_msg, name, buffer);
}
