#include <iostream>
#include <winsock2.h>
#include <string>
#include <conio.h>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable: 4996)

SOCKET ConnectionSocket;
boolean WriteMode = false;
std::string MsgStore;

void SendMsg(std::string msg, SOCKET connectionSocket) {
	int msgSize = msg.size();
	if (send(connectionSocket, (char*)&msgSize, sizeof(int), NULL) == SOCKET_ERROR) {
		std::cout << "size send failed: " << WSAGetLastError() << std::endl;
		return;
	}
	if (send(connectionSocket, msg.c_str(), msgSize, NULL) == SOCKET_ERROR) {
		std::cout << "message send failed: " << WSAGetLastError() << std::endl;
		return;
	}
}

void ServerHandler() {
	int msgSize;
	int recvCode;
	while (true)
	{
		recvCode = recv(ConnectionSocket, (char*)&msgSize, sizeof(int), NULL);
		if (recvCode > 0) {
			char* msg = new char[msgSize + 1];
			msg[msgSize] = '\0';
			recvCode = recv(ConnectionSocket, msg, msgSize, NULL);
			if (recvCode > 0) {
				if (!WriteMode) {
					std::cout << msg << std::endl;
				}
				else {
					MsgStore += msg;
					MsgStore += "\n";
				}
				delete[] msg;
			}
			else if (recvCode == 0) {
				std::cout << "Conncection closed." << WSAGetLastError() << std::endl;
				return;
			}
			else {
				std::cout << "message receive failed: " << WSAGetLastError() << std::endl;
				return;
			}
		}
		else if (recvCode == 0) {
			std::cout << "Conncection closed." << WSAGetLastError() << std::endl;
			return;
		}
		else {
			std::cout << "size receive failed: " << WSAGetLastError() << std::endl;
			return;
		}
	}
}

int main(int argc, char* argv[]) {
	//WSAStartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error: Failed to load library." << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeofAddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	ConnectionSocket = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(ConnectionSocket, (SOCKADDR*)&addr, sizeofAddr) != 0) {
		std::cout << "Error: Failed to connect to the server." << std::endl;
		return 1;
	}
	std::cout << "Successfully connected to the server." << std::endl;

	std::string clientName;
	std::string reciveMsg;
	int reciveMsgSize;
	int recvCode;
	while(reciveMsg != "Welcome to the chat!") {
		std::cout << "Enter your name:" << std::endl;
		std::getline(std::cin, clientName);
		std::string checkSpaceClientName = clientName;
		size_t i;
		while ((i = checkSpaceClientName.find(' ')) != std::string::npos)
		{
			checkSpaceClientName.erase(i, 1);
		}
		if (!checkSpaceClientName.empty()) {
			if (clientName.find(":") == std::string::npos) {
				SendMsg(clientName, ConnectionSocket);
				recvCode = recv(ConnectionSocket, (char*)&reciveMsgSize, sizeof(int), NULL);
				if (recvCode > 0) {
					char* msg = new char[reciveMsgSize + 1];
					msg[reciveMsgSize] = '\0';
					recvCode = recv(ConnectionSocket, msg, reciveMsgSize, NULL);
					if (recvCode > 0) {
						reciveMsg = msg;
						std::cout << reciveMsg << std::endl;
						delete[] msg;
					}
					else if (recvCode == 0) {
						std::cout << "Conncection closed." << WSAGetLastError() << std::endl;
						return 1;
					}
					else {
						std::cout << "message receive failed: " << WSAGetLastError() << std::endl;
						return 1;
					}
				}
				else if (recvCode == 0) {
					std::cout << "Conncection closed." << WSAGetLastError() << std::endl;
					return 1;
				}
				else {
					std::cout << "size receive failed: " << WSAGetLastError() << std::endl;
					return 1;
				}
			}
			else {
				std::cout << "Your name has forbidden symbol - \":\"!" << std::endl;
			}
		}
		else
		{
			std::cout << "Your name is empty!" << std::endl;
		}
	}
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ServerHandler, NULL, NULL, NULL);

	std::string clientMsg;
	char currentButton;
	while (true)
	{
		currentButton = getch();
		switch (currentButton) {
		case 'w':
			WriteMode = true;
			std::cout << "Read your message:" << std::endl;
			std::getline(std::cin, clientMsg);
			std::cout << MsgStore;
			MsgStore.clear();
			WriteMode = false;
			SendMsg(clientMsg, ConnectionSocket);
			Sleep(1);
			break;
		case 's':
			std::cout << "Goodbye. Glad to see you again." << std::endl;
			SendMsg("/!/stop", ConnectionSocket);
			return 0;
		default:
			break;
		}
	}

	return 0;
}