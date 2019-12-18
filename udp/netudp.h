#ifndef NETUDP
#define NETUDP

#define DEF_PORT "8888"
#define DEF_IP "127.0.0.1"

void netudp_bind_server(int *ssock, char *port);
void netudp_rebind_server(int *ssock);
void netudp_bind_client(int *ssock, char *server, char * port);
void netudp_send_packet(int *ssock, struct sockaddr* dest, 
        packetbuffer_t ** packet_out, int * length);
		
#endif