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

//Протокол: 4 байта (размер сообщения), затем само сообщение
//Использовать лучше двусвязные списки (Указатель на предшественника и потомка)

//Номер порта и hostname по умолчанию
#define DEF_PORT 8888
#define DEF_IP "127.0.0.1"

//Имя пользователя и размер сообщения в байтах
#define NAME_LENGTH 16 
#define MESS_SIZE 4 

//Структура клиента (а-ля двусвязный список - т.к. есть ссылка на предка и потомка)
typedef struct ClientStruct {
    int sock; //Сокет
    struct sockaddr_in *sockaddr; //Структура с параметрами (режим, номер порта и т.п.)
    bool connected; //Подключен ли ещё клиент?
    char *name; //Имя клиента (не больше, чем NAME_LENGTH!)
    //Ссылки на предыдущего и на следующего клиентов (а-ля двусвязный список)
    struct ClientStruct *prev; //Ссылка на предка
    struct ClientStruct *next; //Ссылка на потомка
} ClientStruct;

//Структура сообщения
typedef struct MessageStruct {
    char *name; //Имя клиента
    char *mess; //Само сообщение
    u_int32_t messSize; //Размер сообщения
} MessageStruct;

//Количество подключенных клиентов
int numberClients = 0; 
//Мьютекс для работы со списком клиентов (чтобы при отключении клиента не было ошибок)
pthread_mutex_t lock;
//Разделитель (между именем и текстом сообщения будет)
char *divider = " :> "; 
//Текущий клиент
ClientStruct *curClient = NULL; //Пока никого нет

void* clientHandler(void* args);
int readN(int sock, char *buf, int len, int flags);

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
    bzero((char *) &listenerInfo, sizeof(listenerInfo));
    //Инициализирую параметры сервера (режим, номер порта и ip-адрес)
    listenerInfo.sin_family = AF_INET;
    listenerInfo.sin_port = htons(port); //Порт либо введённый в консоли, либо по умолчанию
    listenerInfo.sin_addr.s_addr = htonl(INADDR_ANY); //Любые внутренние адреса
    //Создаю прослушивающий сокет с обработкой возможных ошибок
    int listener = socket(AF_INET, SOCK_STREAM, 0 );
    if (listener < 0) {
        perror("Невозможно создать прослушивающий сокет!\n");
        exit(1); 
    }
    //Настраиваю сокет 
    //Взято здесь: http://qaru.site/questions/380399/solsocket-in-getsockopt
    int tr = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(tr));
    //Прикрепляю сокет к адресу с обработкой возможных ошибок
    int res = bind(listener,(struct sockaddr *)&listenerInfo, sizeof(listenerInfo)); 
    if (res < 0) {
        perror("Ошибка прикрепления сокета к адресу (bind())!\n");
        exit(1); 
    }
    printf("Сервер запущен. Порт № %i\n", port);

    //Инициализирую мьютекс
    if (pthread_mutex_init(&lock, NULL) != 0) {
        perror("Ошибка создания мьютекса!\n");
        exit(-1);
    }
    listen(listener, 32);
    //Принимаем клиентов в бесконечном цикле
    for (;;) {
	//Получение клиентских сокетов
	struct sockaddr_in client_addr;
        unsigned clientLen = sizeof(client_addr);
        int resListener = accept(listener, (struct sockaddr *) &client_addr, &clientLen);
	//Обработка ошибок
	if (resListener < 0) {
	   printf("Ошибка получения сокета!\n");
	   exit(-1);
	}

	//Выделяем память под клиента
	ClientStruct *client = (struct ClientStruct *) malloc (sizeof(ClientStruct));
	//Инициализируем эту память
	*client = (struct ClientStruct) {resListener, &client_addr, tr, NULL, NULL, NULL};

	//Включаем нового клиента в текущий список клиентов
	//Блокируем мьютекс
	pthread_mutex_lock(&lock);
	numberClients = numberClients + 1;
	//Если клиент первый
	if (curClient == NULL) {
	    curClient = client;
	//Если клиенты уже есть
	} else {
	    curClient->next = client;
	    client->prev = curClient;
	    curClient = client;
	}
	//Разблокируем мьютекс
	pthread_mutex_unlock(&lock);

	//Создаем поток под клиентское подключение (с обработкой ошибок)
	pthread_t thrd;
	res = pthread_create(&thrd, NULL, (void *) &clientHandler, client);
        if (res) {
            printf("Ошибка создания нового потока!\n");
        }
    }
    return 0; 
}

//Читаем сообщения от клента (отдельный поток)
void* clientHandler(void* args) {
      ClientStruct *client = (ClientStruct *) args;
      ssize_t num; //Сколько считали
      //Сначала считываем имя клиента
      char readName[NAME_LENGTH];
      size_t divider_len = strlen(divider);
      num = readN(client->sock, &readName[0], NAME_LENGTH, 0);
      client->name = &readName[0];
      if (num > 0) {
	 //Сообщаем, что подключился клиент
	//Вывод адреса (сдвиги на 8, 16 и 32) взят отсюда (последний ответ) -> https://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c 
	 __uint32_t client_addr = client->sockaddr->sin_addr.s_addr;
	 printf("%s (hostname: %d.%d.%d.%d)", client->name, (u_char) client_addr, (u_char) (client_addr >> 8),
           (u_char) (client_addr >> 16), (u_char) (client_addr >> 24)); //Выведется имя и адрес
    	 printf(" connected\n");
	//Пока клиент подключен (флаг в структуре true)
	 while (client->connected) {
	     //Считываем сообщение
	     ssize_t num2;
	     size_t mesSize = 0;
	     num2 = readN(client->sock, (char *) &mesSize, MESS_SIZE, 0);
	     //Проверяем, жив ли клиент
	     if (num2 <= 0) {
		//Если нет - сообщаем, что отключился
		//Вывод адреса (сдвиги на 8, 16 и 32) взят отсюда (последний ответ) -> https://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c 
		 __uint32_t client_addr = client->sockaddr->sin_addr.s_addr;
	 	printf("%s (hostname: %d.%d.%d.%d)", client->name, (u_char) client_addr, (u_char) (client_addr >> 8),
           (u_char) (client_addr >> 16), (u_char) (client_addr >> 24)); //Выведется имя и адрес
    	 	printf(" disconnected\n");
		//А затем - убираем его из списка клиентов
		//Если умерший - текущий, то текущим делаем его предшественника
		if (curClient == client) {
        		curClient = curClient->prev;
    		}
		//Если есть предшественник, то меняем его на потомка
    		if (client->prev != NULL) {
        		client->prev->next = client->next;
    		}
		//Если есть потомок, то меняем на предшественника
    		if (client->next != NULL) {
        		client->next->prev = client->prev;
    		}
		//Блокируем мьютекс
    		pthread_mutex_lock(&lock);
		//Ставим флаг того, что он не подключен
   		client->connected = false;
		//Закрываем сокет для этого клиента
    		close(client->sock);
		//Уменьшаем количество клиентов
    		numberClients= numberClients-1;
		//Разблокируем мьютекс
    		pthread_mutex_unlock(&lock);

	    //Иначе клиент жив
	    } else { 
		char *mess = malloc(sizeof(char) * (mesSize + strlen(client->name) + divider_len));
		//Записываем в сообщение имя и разделитель
		strcpy(mess, client->name);
		strcat(mess, divider);
		//Выделяю память под сообщение
		MessageStruct *messStr = (struct MessageStruct *) malloc(sizeof(struct MessageStruct));
		*messStr = (struct MessageStruct) {client->name, &mess[0], (uint32_t) (mesSize + strlen(client->name) + divider_len)};
		//Читаем столько символов, сколько в заголовке сообщения и записываем
		readN(client->sock, &mess[strlen(client->name) + divider_len], mesSize, 0);
        	printf("%s\n", mess);

		//Рассылаем сообщение всем клиентам
		//Блокирую мьютекс
        	pthread_mutex_lock(&lock);
		//Завожу новую структуру, чтобы не "портить" структуру текущего клиента и не потерять его
    		ClientStruct *receiver = curClient; 
    		//Отправляем сообщения клиентам в обратном порядке (от текущего клиента)
    		while (receiver != NULL) {
		   //Посылаем всем, кроме самого себя
        	   if (receiver->name != messStr->name) {
		      //Cначала пишем размер сообщения
            	      write(receiver->sock, &(messStr->messSize), MESS_SIZE); 
            	      write(receiver->sock, messStr->mess, messStr->messSize);
        	   }
		   //Меняю получателя на предшественника
        	   receiver = receiver->prev;
    		}
		//Разблокирую мьютекс
    		pthread_mutex_unlock(&lock);
		//Очищаю память
    		free(messStr->mess);
    		free(messStr);
	     }
	}
	//Очищаем память
	free(client);
     } else {
     	//Убираем его из списка клиентов
	//Если умерший - текущий, то текущим делаем его предшественника
	     if (curClient == client) {
        	curClient = curClient->prev;
    	     }
	     //Если есть предшественник, то меняем его на потомка
    	     if (client->prev != NULL) {
        	client->prev->next = client->next;
    	     }
	     //Если есть потомок, то меняем на предшественника
    	     if (client->next != NULL) {
        	client->next->prev = client->prev;
    	     }
	     //Блокируем мьютекс
    	     pthread_mutex_lock(&lock);
	     //Ставим флаг того, что он не подключен
   	     client->connected = false;
	     //Закрываем сокет для этого клиента
    	     close(client->sock);
	     //Уменьшаем количество клиентов
    	     numberClients= numberClients-1;
	     //Разблокируем мьютекс
    	     pthread_mutex_unlock(&lock);
     //Очищаем память
     free(client);
     }
}

//Функция для чтения ровно N байт
//Данный вариант найден здесь: http://www.informit.com/articles/article.aspx?p=169505&seqNum=9
int readN(int sock, char *buf, int len, int flags) {
    int curLen; //Текущая длина сообщения
    int recvMy;
    //В первый раз длина та, какую получила функция
    curLen = len; 
    while (curLen > 0) {
        recvMy = recv(sock, buf, curLen, flags);
        if (recvMy < 0) {
	    //Если ошибка прерывания, то читаем снова
	    //Код ошибки прерывания (EINTR) взять здесь: https://ru.wikipedia.org/wiki/Errno.h
            if (errno == EINTR)    
                continue;
	    //Если нет - ошибка          
            return -1;              
        }
	//Если дочитали до конца
        if (recvMy == 0)                
            return (len - curLen); 
        buf = buf + recvMy;
        curLen = curLen - recvMy;
    }
    //Вернем столько, сколько прочитали
    return len;
}
