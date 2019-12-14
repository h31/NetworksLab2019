#ifndef PROTOCOL_H
#define PROTOCOL_H

/* 
    ========= MESSAGE FORMAT =========
    <yyyy-MM-dd hh:mm:ss> [name] : msg
    ==================================
*/ 

#define MIN_NAME_LEN 3      // min user's name len 
#define MAX_NAME_LEN 16     // max user's name len 
#define MAX_MSG_LEN 256     // user's msg len
#define FULL_MSG_LEN 300    // user's msg len + time len + name len + msg additional symbols


int read_msg(int sock, char* buffer);
int send_msg(int sock, char* buffer);
void build_msg(char* full_msg, char* name, char* buffer);

#endif