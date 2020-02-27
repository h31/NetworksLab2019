
#ifndef _H_UDP_RW
#define _H_UDP_RW


#include <stdint.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

// Пересылаем дейтаграммы размером 32 байта
#define DATAGRAM_DATA_LEN (32)
#define DATAGRAM_LEN      (4+DATAGRAM_DATA_LEN)
#define ACK_LEN           (4+4)

#define RETRY_NUMBER     5
#define RETRY_TIMEOUT_MS 1000

// READ N BYTES FROM FILE DESCRIPTOR INTO CHAR BUFFER
int custom_read(int socket_fd, struct sockaddr_in *addr, unsigned int addr_len, char *buf, const unsigned int buf_len);

// WRITE N BYTES FROM CHAR BUFFER INTO FILE DESCRIPTOR
int custom_write(int socket_fd, struct sockaddr_in *addr, unsigned int addr_len, char *buf, const int buf_len);


#endif // _H_UDP_RW
