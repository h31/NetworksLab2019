
#include "udp.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <poll.h>

int custom_read(int socket_fd, struct sockaddr_in *addr, unsigned int addr_len, char *buf, const unsigned int buf_len)
{
	int sum = 0;
	int n;
	char dgram[DATAGRAM_LEN];
	const int32_t *intvec = (int32_t*)dgram;
	char *dgram_data = (char*)(intvec+1);
	
	const unsigned int n_left = (buf_len%DATAGRAM_DATA_LEN) ? (buf_len/DATAGRAM_DATA_LEN + 1) : (buf_len/DATAGRAM_DATA_LEN);
	bool received[n_left]; // If each datagram was received or not
	bzero(received, sizeof(received));
	
	const char ack[4] = "ack";
	
	while (sum < buf_len)
	{
		int adlen = addr_len;
		n = recvfrom(socket_fd, dgram, DATAGRAM_LEN, 0, (struct sockaddr*)addr, &adlen);
		if (n <= 0) return n;
		if (n < DATAGRAM_LEN) return -1;
		
		printf("Datagram [%d] - %d bytes\n", intvec[0], n);
		
		if (!received[intvec[0]])
		{
			// If we received new part of buffer
			received[intvec[0]] = true;
			strncpy(buf + (intvec[0] * DATAGRAM_DATA_LEN), dgram_data, DATAGRAM_DATA_LEN);
			sum += DATAGRAM_DATA_LEN;
		}
		
		// Send ack
		strncpy(dgram_data, ack, strlen(ack)+1);
		n = sendto(socket_fd, dgram, ACK_LEN, 0, (struct sockaddr*)addr, addr_len);
		printf("ACK sent\n");
	}
	return sum;
}

int custom_write(int socket_fd, struct sockaddr_in *addr, unsigned int addr_len, char *buf, const int buf_len)
{
	int sum = buf_len;
	int n;
	unsigned int n_timeouts = RETRY_NUMBER;
	char dgram[DATAGRAM_LEN];
	int32_t *intvec = (int32_t*)dgram;
	char *dgram_data = (char*)(intvec+1);
	
	for (unsigned int i = 0; sum > 0 && n_timeouts > 0;)
	{
		intvec[0] = i;
		
		// Copy data into tx buffer
		strncpy(dgram_data, buf + (i * DATAGRAM_DATA_LEN), DATAGRAM_DATA_LEN);
		
		// Send datagram
		n = sendto(socket_fd, dgram, DATAGRAM_LEN, 0, (struct sockaddr*)addr, addr_len);
		
		// Read ack
		struct pollfd pfd = {.fd = socket_fd, .events = POLLIN};
		int pollstatus = poll(&pfd, 1, RETRY_TIMEOUT_MS);
		if (pollstatus == -1) printf("Poll error\n");
		if (pollstatus == 0)
		{
			// Timeout
			--n_timeouts;
			continue; // Send datagram again
		}
		else
		{
			// Ack received
			int adlen = addr_len;
			n = recvfrom(socket_fd, dgram, DATAGRAM_LEN, 0, (struct sockaddr*)addr, &adlen);
			if (n == 8 && intvec[0] == i && !strcmp(dgram_data, "ack"))
			{
				printf("ACK received\n");
				sum -= DATAGRAM_DATA_LEN;
				++i; // Go to next datagram
				n_timeouts = RETRY_NUMBER;
			}
			else
			{
				continue; // Send datagram again
			}
		}
	}
	
	return buf_len - sum;
}

