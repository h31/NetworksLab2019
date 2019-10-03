CC = gcc
CFLAGS = -g -pthread

all:
	$(CC) $(CFLAGS) -o client/client_linux client/main.c
	$(CC) $(CFLAGS) -o server/server_linux server/main.c

clean:
	rm -f client/client_linux server/server_linux
