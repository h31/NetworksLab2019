#ifndef course
#define course

#define msg_length 256
#define max_clients 50
#define CNAME_SIZE 30	//название страны
#define FNAME_SIZE 3		//название валют
#define MAX_FID 30
#define HIS_NUM 10
#define TEMPID_SIZE 5
#define TEMPC_SIZE 21
#define EXIT "/exit"
#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"
#define how_shutdown 2 	//зарпещена передача данных

int clients_sockets[max_clients];
pthread_t clients_threads[max_clients];
pthread_mutex_t mp;

struct Finance_data
{
	int FID; 									//ID валюты
	char cname[CNAME_SIZE];	//название страны
	char name[FNAME_SIZE];	// имя валюты
	float Course; 							//Курс валюты
	float conCourse; 					//Относительное приращение курса
	float absCourse; 					//Абсолютное приращение курса
	int HistoryCounter;
	char FHistory[HIS_NUM][21];
};

struct Finance_data FData[MAX_FID];//Массив из структур типа Finance_data

void* acceptHandler(void* arg);
void* clientHandler(void* arg);
void killClient(pthread_t tid, int server);

void nettcp_bind_server(int *server, int port);
int readN(int sock, char* buf, int length, int flags);
int readFix(int sock, char* buf, int bufSize, int flags);
int sendFix(int sock, char* buf);
void close_client(int sock); 
int alive_client (int sock);

int findGivenID (int givenID);
int parseDataFromCmd(int argsNumb, char msg_buf[msg_length]);
void sendData(int ID, char cmd, int sock);
void sendClientConfirm(int cli_sock, bool flag);
int strCmpWithoutSpaces(char* static_str, char* final_str);
#endif