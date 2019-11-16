//
// Created by Malik Hiraev on 19.10.2019.
//

#ifndef NETWORKSLAB2019_READN_H
#define NETWORKSLAB2019_READN_H

#include <sys/socket.h>
#include <errno.h>
#define USER_NAME_SIZE 32
// Message size in bytes (Размер числа для обозначения размера сообщения 4 байта - int)
#define MSG_SIZE_VAL 4

int readn(int fd, char *bp, size_t len);

void replace(char *buff, size_t size, char old, char new);

#endif //NETWORKSLAB2019_READN_H
