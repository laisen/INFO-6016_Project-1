#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "..//Buffer/Protocol.h"

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 256
#define DEFAULT_PORT "1234"

// Client structure
struct ClientInfo {
	SOCKET socket;

	// Buffer information (this is basically you buffer class)
	WSABUF dataBuf;
	char buffer[DEFAULT_BUFLEN];
	int bytesRECV;
};

int TotalClients = 0;
ClientInfo* ClientArray[FD_SETSIZE];

//void RemoveClient(int index)
//{
//	ClientInfo* client = ClientArray[index];
//	closesocket(client->socket);
//	printf("Closing socket %d\n", (int)client->socket);
//
//	for (int clientIndex = index; clientIndex < TotalClients; clientIndex++)
//	{
//		ClientArray[clientIndex] = ClientArray[clientIndex + 1];
//	}
//
//	TotalClients--;
//
//	// We also need to cleanup the ClientInfo data
//	// TODO: Delete Client
//}

int main(int argc, char** argv)
{
	WSADATA wsaData;
	int iResult;	

	//int iSendResult;
	char recvBytes[DEFAULT_BUFLEN];
	int recvBytesLen = DEFAULT_BUFLEN;

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

	// #1 Socket
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET acceptSocket = INVALID_SOCKET;

	struct addrinfo* addrResult = NULL;
	struct addrinfo hints;

	// Define our connection address info 
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addrResult);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("getaddrinfo() is good!\n");
	}

	// Create a SOCKET for connecting to server
	listenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (listenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(addrResult);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("socket() is created!\n");
	}

	// #2 Bind - Setup the TCP listening socket
	iResult = bind(listenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(addrResult);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("bind() is good!\n");
	}

	freeaddrinfo(addrResult);

	// #3 Listen
	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("listen() was successful!\n");
	}

	DWORD NonBlock = 1;
	iResult = ioctlsocket(listenSocket, FIONBIO, &NonBlock);
	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	printf("ioctlsocket() was successful!\n");
	
	FD_SET ReadSet;
	FD_ZERO(&ReadSet);

	FD_SET(listenSocket, &ReadSet);

	while (true)
	{
		fd_set copy = ReadSet;
		
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; i++)
		{			
			SOCKET sock = copy.fd_array[i];
			
			if (sock == listenSocket)
			{
				SOCKET client = accept(listenSocket, nullptr, nullptr);
				
				FD_SET(client, &ReadSet);				
			}
			else
			{				
				int bytesReceived = recv(sock, recvBytes, recvBytesLen, 0);
				std::cout << recvMessage(recvBytes, bytesReceived) << std::endl;
				for (int i = 0; i < ReadSet.fd_count; i++)
				{
					SOCKET outSock = ReadSet.fd_array[i];
					if (outSock != listenSocket && outSock != sock)
					{
						send(outSock, recvBytes, bytesReceived, 0);
					}
				}				
			}
		}
	}

	FD_CLR(listenSocket, &ReadSet);
	closesocket(listenSocket);

	// #6 close
	iResult = shutdown(acceptSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(acceptSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	WSACleanup();
	system("Pause");
	return 0;
}