#include <iostream>
#include <winsock2.h>
#include <string>
#include <map>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

std::map <SOCKET, std::string> Clients;
int MaxClients = 120;

void ExitClient(SOCKET connectionSocket) {
	if (Clients.find(connectionSocket) != Clients.end()) {
		std::cout << "Client \"" << Clients.find(connectionSocket)->second << "\" disconnect from the chat." << std::endl;
		Clients.erase(connectionSocket);
	}
	closesocket(connectionSocket);
}

void SendMsg(std::string msg, SOCKET connectionSocket) {
	int msgSize = msg.size();
	if (send(connectionSocket, (char*)&msgSize, sizeof(int), NULL) == SOCKET_ERROR) {
		std::cout << "size send failed: " << WSAGetLastError() << std::endl;
		ExitClient(connectionSocket);
		return;
	}
	if (send(connectionSocket, msg.c_str(), msgSize, NULL) == SOCKET_ERROR) {
		std::cout << "message send failed: " << WSAGetLastError() << std::endl;
		ExitClient(connectionSocket);
		return;
	}
}

void ClientHandler(SOCKET connectionSocket) {
	int msgSize;
	int recvCode;
	boolean nameFactor = false;
	while (!nameFactor) {
		recvCode = recv(connectionSocket, (char*)&msgSize, sizeof(int), NULL);
		if (recvCode > 0) {
			char* msg = new char[msgSize + 1];
			msg[msgSize] = '\0';

			recvCode = recv(connectionSocket, msg, msgSize, NULL);
			if (recvCode > 0) {
				boolean repeatNameFactor = false;
				for (auto client : Clients) {
					if (client.second == msg) {
						SendMsg("This name already used by other user!", connectionSocket);
						repeatNameFactor = true;
						//continue;
					}
				}
				if (!repeatNameFactor) {
					std::cout << "Client \"" << msg << "\" connect to the chat." << std::endl;
					SendMsg("Welcome to the chat!", connectionSocket);
					Clients.emplace(connectionSocket, msg);
					nameFactor = true;
				}
				delete[] msg;
			}
			else if (recvCode == 0) {
				std::cout << "Conncection closed." << WSAGetLastError() << std::endl;
				ExitClient(connectionSocket);
				return;
			}
			else {
				std::cout << "message receive failed: " << WSAGetLastError() << std::endl;
				ExitClient(connectionSocket);
				return;
			}
		}
		else if (recvCode == 0) {
			std::cout << "Conncection closed." << WSAGetLastError() << std::endl;
			ExitClient(connectionSocket);
			return;
		}
		else {
			std::cout << "size receive failed: " << WSAGetLastError() << std::endl;
			ExitClient(connectionSocket);
			return;
		}
	}
	
	std::string clientName = Clients.find(connectionSocket)->second;
	time_t seconds;
	tm* timeinfo;
	const char* format = "(%H:%M:%S) ";
	char date[12];
	while (true)
	{
		recvCode = recv(connectionSocket, (char*)&msgSize, sizeof(int), NULL);
		if (recvCode > 0) {
			char* msg = new char[msgSize + 1];
			msg[msgSize] = '\0';
			recvCode = recv(connectionSocket, msg, msgSize, NULL);			
			if (recvCode > 0) {
				if (strcmp(msg, "/!/stop") == 0) {
					ExitClient(connectionSocket);
					delete[] msg;
					return;
				}
				else
				{
					seconds = time(NULL);
					timeinfo = localtime(&seconds);
					strftime(date, 12, format, timeinfo);
					std::string updateMsg = date;
					updateMsg += clientName;
					updateMsg += ": ";
					updateMsg += msg;
					for (auto client : Clients) {
						if (client.second != clientName) {
							SendMsg(updateMsg, client.first);
						}
					}
				}
				delete[] msg;
			}
			else if (recvCode == 0) {
				std::cout << "Conncection closed." << WSAGetLastError() << std::endl;
				ExitClient(connectionSocket);
				return;
			}
			else {
				std::cout << "message receive failed: " << WSAGetLastError() << std::endl;
				ExitClient(connectionSocket);
				return;
			}
		}
		else if (recvCode == 0) {
			std::cout << "Conncection closed." << WSAGetLastError() << std::endl;
			ExitClient(connectionSocket);
			return;
		}
		else {
			std::cout << "size receive failed: " << WSAGetLastError() << std::endl;
			ExitClient(connectionSocket);
			return;
		}
	}
}

int main(int argc, char* argv[]) {
	//WSAStartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "WSAStartup failed." << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofAddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, NULL);
	bind(listenSocket, (SOCKADDR*)&addr, sizeofAddr);
	listen(listenSocket, SOMAXCONN);

	SOCKET newConnectionSocket;
	while (Clients.size() < MaxClients) {
		newConnectionSocket = accept(listenSocket, (SOCKADDR*)&addr, &sizeofAddr);
		if (newConnectionSocket == 0) {
			std::cout << "Client cannot connect to the server." << std::endl;
		}
		else
		{
			std::cout << "Client successfully connected to the server." << std::endl;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(newConnectionSocket), NULL, NULL);
		}
	}
	return 0;
}