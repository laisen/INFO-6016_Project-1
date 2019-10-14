#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "..//Buffer/Protocol.h"
#include <conio.h>

#define ESCAPE 27
#define DELETE 8
#define CARRIAGE_RETURN 13

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")


#define DEFAULT_BYTESLEN 512
#define DEFAULT_PORT "5150"

int main(int argc, char** argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	//const char* sendbuf = "this is a test";
	char recvBytes[DEFAULT_BYTESLEN];
	int iResult;
	int recvBytesLen = DEFAULT_BYTESLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}
	else
	{
		printf("WSAStartup() was successful!\n");
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("getaddrinfo() successful!\n");
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL;ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	printf("Successfully connected to the server on socket %d!\n", (int)ConnectSocket);

	std::cout << "Please input your name:" << std::endl;
	std::cout << "> ";
	std::string nameInput;
	getline(std::cin, nameInput);

	std::cout << "Please input the room number you would like to join:" << std::endl;
	std::cout << "1 - room#1" << std::endl;
	std::cout << "2 - room#2" << std::endl;
	std::cout << "3 - room#3" << std::endl;
	std::cout << "> ";
	int numInput;
	std::string roomName;
	std::cin >> numInput;
	std::cin.ignore();
	switch (numInput)
	{
	case 1:
		roomName = "room#1";
		break;
	case 2:
		roomName = "room#2";
		break;
	case 3:
		roomName = "room#3";
		break;
	default:
		std::cout << "Invalid input" << "\n";
		break;
	}	

	send(ConnectSocket, joinRoom(roomName, nameInput).c_str(), joinRoom(roomName, nameInput).size(), 0);

	//ZeroMemory(recvBytes, recvBytesLen);
	int bytesReceived = recv(ConnectSocket, recvBytes, recvBytesLen, 0);
	std::cout << recvMessage(recvBytes, bytesReceived) << std::endl;

	std::cout << "[" << roomName << "] [" << nameInput << "] " << "> ";
	std::string messageInput;
	do
	{
		// Prompt the user for some text
		//std::cout << "[" << roomName << "] [" << nameInput << "] "<< "> ";
		//getline(std::cin, messageInput);

		int bytesReceived = recv(ConnectSocket, recvBytes, recvBytesLen, 0);
		if (bytesReceived > 0)
		{					
			// Echo response to console
			std::cout << recvMessage(recvBytes, bytesReceived) << std::endl;
		}

		if (_kbhit())
		{
			char ch = _getch();
			if (ch == DELETE)
			{
				messageInput.pop_back();
				printf("\b \b");
			}
			else
			{
				messageInput.push_back(ch);
				printf("%c", ch);
			}


			if (ch == CARRIAGE_RETURN)
			{
				iResult = send(ConnectSocket, sendMessage(roomName, nameInput, messageInput).c_str(),
					sendMessage(roomName, nameInput, messageInput).size(), 0);
				messageInput.clear();
				printf("\n");
				std::cout << "[" << roomName << "] [" << nameInput << "] " << "> ";
			}

			if (ch == ESCAPE)
			{
				iResult = send(ConnectSocket, leaveRoom(roomName, nameInput).c_str(), leaveRoom(roomName, nameInput).size(), 0);
				break;
			}
		}

		//if (messageInput.size() > 0)		// Make sure the user has typed in something
		//{			
		//	if (messageInput == "/leave")
		//	{
		//		iResult = send(ConnectSocket, leaveRoom(roomName, nameInput).c_str(), leaveRoom(roomName, nameInput).size(), 0);
		//		break;
		//	}
		//	// Send the text
		//	iResult = send(ConnectSocket, sendMessage(roomName, nameInput, messageInput).c_str(),
		//		sendMessage(roomName, nameInput, messageInput).size(), 0);
		//	//if (iResult != SOCKET_ERROR)
		//	//{
		//	//	// Wait for response
		//	//	ZeroMemory(recvBytes, recvBytesLen);
		//	//	int bytesReceived = recv(ConnectSocket, recvBytes, recvBytesLen, 0);
		//	//	if (bytesReceived > 0)
		//	//	{					
		//	//		// Echo response to console
		//	//		std::cout << recvMessage(recvBytes, bytesReceived) << std::endl;
		//	//	}
		//	//}
		//}

	} while (messageInput.size() >= 0);

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	printf("\nSSuccessfully shutdown socket %d!\n", (int)ConnectSocket);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();
	system("Pause");
	return 0;
}