#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <netdb.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/ioctl.h>

//Протокол: 4 байта (размер сообщения), затем само сообщение

//Номер порта и hostname по умолчанию
#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"

//Имя пользователя и размер сообщения в байтах
#define NAME_LENGTH 16 
#define MESS_SIZE 4
#define MAX_CLIENTS 1024 
//Когда блокируется 
#define POLL_TIMEOUT (-1)

//Тип, обозначающий, чего ждет клиент (про enum в си читала тут - https://learnc.info/c/enums.html)
enum ClientWaiting {
	FORNAME, FORMESSAGE, FORSIZE
};

//Структура сообщения
typedef struct MessageStruct {
    char *mess; //Само сообщение
    u_int32_t messSize; //Размер сообщения
} MessageStruct;

//Структура клиента (а-ля двусвязный список - т.к. есть ссылка на предка и потомка)
typedef struct ClientStruct {
    int sock; //Сокет
    struct sockaddr_in *sockaddr; //Структура с параметрами (режим, номер порта и т.п.)
    enum ClientWaiting clientWaiting; //Статус клиента - чего ждет?
    unsigned bytes; // Сколько еще надо прочитать байт?
    MessageStruct *msg;	//Само сообщение
    char *name; //Имя клиента (не больше, чем NAME_LENGTH!)
    //Ссылки на предыдущего и на следующего клиентов (а-ля двусвязный список)
    struct ClientStruct *prev; //Ссылка на предка
    struct ClientStruct *next; //Ссылка на потомка
} ClientStruct;

//Количество подключенных клиентов
int numberClients = 0; 
//Первый и последний клиенты
ClientStruct *first = NULL;
ClientStruct *last = NULL;
//Разделитель (между именем и текстом сообщения будет)
char *divider = " :> ";

int listener;

//Синтаксис смотрела тут: https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_72/apis/poll.htm
struct pollfd fds[1+MAX_CLIENTS]; //Файловые дескрипторы
unsigned fdsLength = 1; //Пока нет клиентов

unsigned last_ind();
void clientHandler();
void closingfd();
void printName(ClientStruct *cl);
void readingMess(ClientStruct *newClient, int j);
bool waitForSize(ClientStruct *newClient, int j);
bool waitForName(ClientStruct *newClient, int j);
bool waitForMess(ClientStruct *newClient, int j);
void sendMessage (ClientStruct *newClient);


//--------------------------BEGIN MAIN---------------------------------------------
int main(int argc, char** argv) {
    //Сперва создать сервер, порт 8888 по умолчанию, а так м.б. аргументом
    uint16_t port = 0;
    //Если аргументы не заданы, то использую порт по умолчанию
    if (argc < 2) {
	printf("Неполный вызов. Необходимо: ./server.o port \n");
        printf("Будет использован порт по умолчанию %d\n", DEF_PORT);
        port = DEF_PORT;
    } else
    	port = (uint16_t) atoi(argv[1]);

    //Инициализирую сервер
    struct sockaddr_in listenerInfo;
    //Создаю прослушивающий сокет с обработкой возможных ошибок
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("Невозможно создать прослушивающий сокет!\n");
        exit(1); 
    }

    //Настраиваю сокет 
    //Взято здесь: http://qaru.site/questions/380399/solsocket-in-getsockopt
    int tr = 1;
    int setsock;
    setsock = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(tr));
    if (setsock < 0) {
	printf("Ошибка настройки сокета - setsockopt!\n");
	close(listener);
	exit(-1);
    }

    //Сокет должен быть неблокирующим: https://stackoverflow.com/questions/9748568/non-blocking-socket-and-io (посмотрела, как сделать, тут)
    setsock = ioctl(listener, FIONBIO, (char *) &tr);
     if (setsock < 0) {
	printf("Ошибка настройки неблокирующего сокета - ioctl!\n");
	close(listener);
	exit(-1);
    }

    bzero((char *) &listenerInfo, sizeof(listenerInfo));
    //Инициализирую параметры сервера (режим, номер порта и ip-адрес)
    listenerInfo.sin_family = AF_INET;
    listenerInfo.sin_port = htons(port); //Порт либо введённый в консоли, либо по умолчанию
    listenerInfo.sin_addr.s_addr = htonl(INADDR_ANY); //Любые внутренние адреса

    //Прикрепляю сокет к адресу с обработкой возможных ошибок
    int res = bind(listener,(struct sockaddr *)&listenerInfo, sizeof(listenerInfo)); 
    if (res < 0) {
        perror("Ошибка прикрепления сокета к адресу (bind())!\n");
        exit(1); 
    }
    printf("Сервер запущен. Порт № %i\n", port);

    listen(listener, 32);
    //Инициализирую структуру fds - Инфу брала тут: https://web-answers.ru/c/opros-soket-programmirovanija-tcp-linux-problema-s.html и тут https://habr.com/ru/post/272269/
    memset(fds, 0, sizeof(fds));
    //Настройка для принимающей стороны
    fds[0].fd = listener;
    fds[0].events = POLLIN;
    
    //Принимаем клиентов в бесконечном цикле
    for (;;) {
	int inner = poll(fds, fdsLength, POLL_TIMEOUT);
	if (inner < 0) {
	    printf("Ошибка вызова poll()!\n");
	    closingfd(); //Функция закрытия дескрипторов
	}
	else {
	    //Получаем дескрипторы
	    int receiveRevents = fds[0].revents;
	    //Если можно считывать данные (https://www.opennet.ru/man.shtml?topic=poll&category=2&russian=0)
	    if (receiveRevents == POLLIN) { 
		clientHandler();
	    }
	    else if (receiveRevents != 0) {
	    	printf("Неверное значение возвращаемого события - revents!\n");
		closingfd(); //Функция закрытия дескрипторов
	    }

	    ClientStruct *newClient = first;
	    while (newClient != NULL) {
		//http://ru.manpages.org/poll/2
		int j = newClient->sock; //чтобы далее использовать в качестве индекса
		struct pollfd *poll_fd = &fds[j];
		//Если клиент "жив" и подключен
		if (poll_fd->revents == POLLIN)
		    readingMess(newClient, j);	//Читаем сообщения
		//Если клиент отключился
		else if (poll_fd->revents != 0) {
    			printName(newClient); // Печатаем, что клиент отключился
    			printf(" disconnected\n");

			struct pollfd pfd2 = fds[j];
			shutdown(pfd2.fd, SHUT_RDWR);
			close(pfd2.fd);
			//Реструктурируем, чтобы не было пустых ячеек в массиве
			unsigned lastInd = last_ind();
			bool notLast = j != lastInd;
			if (notLast)
			   memccpy(&fds[j], &fds[lastInd], 0, sizeof(struct pollfd));
			fdsLength = fdsLength-1;

			if (first == newClient) {
			    first = first->next;
			}
			if (newClient->prev != NULL) {
			   newClient->prev->next = newClient->next;
			}
			if (newClient->next != NULL) {
			   newClient->next->prev = newClient->prev;
			}
			if (newClient != last) {
			   last->sock = newClient->sock;
			}
			else {
			   last = last->prev;
			}
			if (newClient->msg != NULL) {
			   if (newClient->msg->mess != NULL)
			   	free(newClient->msg->mess);
			   free(newClient->msg);
			}
			if (newClient->name != NULL)
			   free(newClient->name);
			free(newClient);
  			numberClients = numberClients-1;
		}
		poll_fd->revents = 0;
		newClient = newClient->next;
	    }
	}
    }
}

//-------------------------------------END MAIN------------------------------------

unsigned last_ind() {
      unsigned tmp = fdsLength - 1;
      return tmp;
}

//Закрываем все файловые дескрипторы - пример читала тут https://stackoverflow.com/questions/51549985/if-i-called-shutdownfd-shut-rdwr-but-not-called-closefd-what-will-happen
void closingfd() {
	for (unsigned i = 0; i < fdsLength; ++i) {
	    shutdown(fds[i].fd, SHUT_RDWR);
	    close(fds[i].fd);
	}
	exit(-1);
}

void printName(ClientStruct *cl) {
    	printf("%s", cl->name); //Выведется имя
}

void readingMess(ClientStruct *newClient, int j) {
	bool flag;
	if (newClient->clientWaiting == FORSIZE) //Если клиент ждёт размера
		flag = waitForSize(newClient, j);
	else if (newClient->clientWaiting == FORNAME) { //Если клиент ждёт имя
		flag = waitForName(newClient, j);
		printName(newClient);
    		printf(" connected\n");
	}
	else //FORMESSAGE - Если клиент ждет сообщения
		flag = waitForMess(newClient, j);
	if (flag) {
		if (newClient->bytes == 0) {
		   switch (newClient->clientWaiting) {
		   case FORSIZE:
			newClient->clientWaiting = FORMESSAGE;
			newClient->bytes = newClient->msg->messSize;
			break;
		   case FORNAME:
			newClient->clientWaiting = FORSIZE;
			newClient->bytes = MESS_SIZE;
			break;
		   case FORMESSAGE:
			newClient->clientWaiting = FORSIZE;
			newClient->bytes = MESS_SIZE;

			sendMessage(newClient); //Отправляем сообщение всем, кроме себя
		   }
		}
	}
	else {
		printName(newClient);
    		printf(" disconnected\n");
	
		struct pollfd pfd2 = fds[j];
		shutdown(pfd2.fd, SHUT_RDWR);
		close(pfd2.fd);
			
		unsigned lastInd = last_ind();
		bool notLast = newClient->sock != lastInd;
		//Если этот клиент не последний подключившийся			
		if (notLast)
		   memccpy(&fds[j], &fds[lastInd], 0, sizeof(struct pollfd));
		fdsLength = fdsLength-1;
		//Реструктурируем, чтобы не было пустых ячеек в массиве
		if (first == newClient)
		    first = first->next;
		if (newClient->prev != NULL)
		   newClient->prev->next = newClient->next;
		if (newClient->next != NULL)
		   newClient->next->prev = newClient->prev;
		if (newClient != last)
		   last->sock = newClient->sock;
		else
		   last = last->prev;
		if (newClient->msg != NULL) {
		   if (newClient->msg->mess != NULL)
		   	free(newClient->msg->mess);
			free(newClient->msg);
		}
		if (newClient->name != NULL)
		   free(newClient->name);
		//Освобождаю память и уменьшаю число клиентов
		free(newClient);
  		numberClients = numberClients-1;
	}
}

bool waitForSize(ClientStruct *newClient, int j) {
	size_t dif = MESS_SIZE - newClient->bytes;
	size_t mess_size = 0;
	ssize_t rd = read((&fds[j])->fd, (char *) &mess_size, newClient->bytes);
	mess_size = newClient->msg->messSize + (mess_size << (8*dif));
	if (rd == 0)
	    return false;
	else {
	   if (rd < newClient->bytes)
	      newClient->bytes = newClient->bytes - rd;
	   else {
	       newClient->msg->mess = malloc(sizeof(char)*mess_size);
	       newClient->bytes = 0;
	   }
	   newClient->msg->messSize = mess_size;
	   return true;
	}
}

bool waitForName(ClientStruct *newClient, int j) {
	size_t dif = NAME_LENGTH - newClient->bytes;
	ssize_t rd = read((&fds[j])->fd, &newClient->name[dif], newClient->bytes);
	if (rd == 0)
	   return false;
	else {
	   if (rd < newClient->bytes)
	      newClient->bytes = newClient->bytes - rd;
	   else 
	      newClient->bytes = 0;
	   return true;
	}
}

bool waitForMess(ClientStruct *newClient, int j) {
	size_t dif = newClient->msg->messSize - newClient->bytes;
	ssize_t rd = read((&fds[j])->fd, &newClient->msg->mess[dif], newClient->bytes);
	if (rd == 0)
	   return false;
	else {
	   if (rd < newClient->bytes)
	      newClient->bytes = newClient->bytes - rd;
	   else 
	      newClient->bytes = 0;
	   return true;
	}
}

void sendMessage (ClientStruct *newClient) {
	MessageStruct *newMess = newClient->msg;
	u_int32_t fullMessSize = (u_int32_t) (newMess->messSize + strlen(newClient->name) + strlen(divider));
	
	char *mesg = malloc(sizeof(char)*fullMessSize);
	//Записываем в сообщение имя и разделитель
	strcpy(mesg, newClient->name);
	strcat(mesg, divider);
	strcat(mesg, newMess->mess);

	MessageStruct *nameMess = (MessageStruct *) malloc(sizeof(MessageStruct));
	*nameMess = (MessageStruct) {mesg, fullMessSize};
	ClientStruct *receiver = first;
	while (receiver != NULL) {
	    if (receiver != newClient) {
		int jr = receiver->sock;
		int sockk = (&fds[jr])->fd;
		write(sockk, &(nameMess->messSize), MESS_SIZE);
		write(sockk, nameMess->mess, nameMess->messSize);
	    }
	    receiver = receiver->next;
	}
	
	//Очищаю память после клиента
	free(newClient->msg->mess);
	free(nameMess->mess);
	free(nameMess);
	newClient->msg->mess = NULL;
	newClient->msg->messSize = 0;
}

void clientHandler() {
     if (numberClients == MAX_CLIENTS)
	return;
     struct sockaddr_in client_addr;
     unsigned client_lenght = sizeof(client_addr);
     int clientListener = accept(listener, (struct sockaddr *) &client_addr, &client_lenght);
     if (clientListener < 0) {
	printf("Ошибка прослушивания клиента - accept()!\n");
	//Закрываем все файловые дескрипторы
	for (unsigned i = 0; i < fdsLength; ++i) {
	    	shutdown(fds[i].fd, SHUT_RDWR);
		close(fds[i].fd);
	}
	exit(-1);
     }
     //Настройка файловых дескрипторов
     int indexOf_fd;
     if (fdsLength == 1+MAX_CLIENTS)
	return;
     else {
	fds[fdsLength].fd = clientListener;
     	fds[fdsLength++].events = POLLIN;
	indexOf_fd = last_ind();
    }
    ClientStruct *client = (ClientStruct *) malloc(sizeof(ClientStruct));
    char *name = (char *) malloc(sizeof(char) * NAME_LENGTH);
    MessageStruct *message = (MessageStruct *) malloc(sizeof(MessageStruct));
    *message = (MessageStruct) {NULL, 0};

    *client = (ClientStruct) {indexOf_fd, &client_addr, FORNAME, NAME_LENGTH, message, name, NULL, NULL};
     //Сообщаем, что подключился клиент
     //Вывод адреса (сдвиги на 8, 16 и 32) взят отсюда (последний ответ) -> https://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c 
    __uint32_t clien_addr = client->sockaddr->sin_addr.s_addr;
    printf("Somebody (hostname: %d.%d.%d.%d)", (u_char) clien_addr, (u_char) (clien_addr >> 8),
    (u_char) (clien_addr >> 16), (u_char) (clien_addr >> 24)); //Выведется имя и адрес
    //printf("%s (hostname: %d.%d.%d.%d)", client->name, (u_char) clien_addr, (u_char) (clien_addr >> 8),
    //(u_char) (clien_addr >> 16), (u_char) (clien_addr >> 24)); //Выведется имя и адрес
    printf(" connected\n");

    //Увеличиваем количество клиентов и проверяем, первый ли он
    numberClients = numberClients+1;
    if (first == NULL) {
        first = client;
    } 
    else {
        last->next = client;
        client->prev = last;
    }
    last = client;
}
