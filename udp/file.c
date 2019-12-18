#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "tftp.h"

int file_open_read(char *filename, int *filedata){
    *filedata = open(filename, O_RDONLY);
    
    if (*filedata == -1){
        fprintf(stderr, "%s\n",strerror(errno));
        return -1;
    }else{
        return 0;
    }
}

int file_buffer_from_pos(transaction_t *transaction){
	//control pointer in file 
    if ((lseek(transaction->filedata, transaction->filepos, SEEK_SET)) == -1){
        fprintf(stderr, "%s\n",strerror(errno));
        return -1;
    }
   
    int bytes_read = read(transaction->filedata,&transaction->filebuffer, BLIMIT);
    
    if (bytes_read == -1) {
        fprintf(stderr, "%s\n",strerror(errno));       
        return -1;
    }
    
    return bytes_read;
}

int file_open_write(char *filename, int *filedata){
    *filedata = open(filename, O_RDWR | O_CREAT, 0666);
    
    if (*filedata == -1){
        fprintf(stderr, "%s\n",strerror(errno));
        return -1;
    }
    
    return 0;
}

int file_append_from_buffer(packet_t *packet, transaction_t *transaction){

    if ((write(transaction->filedata, packet->data, packet->data_length)) == -1){
        fprintf(stderr, "%s\n",strerror(errno));
        return -1;
    }
    
    return 0;
}

int file_close(int *filedata){

    if ((close(*filedata)) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        return -1;
    }else{
        return 0;
    }
}