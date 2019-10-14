#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "..//Buffer/Protocol.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 256
#define DEFAULT_PORT "5150"

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

void RemoveClient(int index)
{
	ClientInfo* client = ClientArray[index];
	closesocket(client->socket);
	printf("Closing socket %d\n", (int)client->socket);

	for (int clientIndex = index; clientIndex < TotalClients; clientIndex++)
	{
		ClientArray[clientIndex] = ClientArray[clientIndex + 1];
	}

	TotalClients--;

	// We also need to cleanup the ClientInfo data
	// TODO: Delete Client
}

int main(int argc, char** argv)
{
	WSADATA wsaData;
	int iResult;	

	//int iSendResult;
	//char recvBytes[DEFAULT_BUFLEN];
	//int recvBytesLen = DEFAULT_BUFLEN;

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

	// Change the socket mode on the listening socket from blocking to
	// non-blocking so the application will not block waiting for requests
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
	int total;
	DWORD flags;
	DWORD RecvBytes;

	printf("Entering accept/recv/send loop...\n");
	while (true)
	{
		timeval tv = { 0 };
		tv.tv_sec = 2;
		// Initialize our read set
		FD_ZERO(&ReadSet);

		// Always look for connection attempts
		FD_SET(listenSocket, &ReadSet);

		// Set read notification for each socket.
		for (int i = 0; i < TotalClients; i++)
		{
			FD_SET(ClientArray[i]->socket, &ReadSet);
		}

		// Call our select function to find the sockets that
		// require our attention
		printf("Waiting for select()...\n");
		total = select(0, &ReadSet, NULL, NULL, &tv);
		if (total == SOCKET_ERROR)
		{
			printf("select() failed with error: %d\n", WSAGetLastError());
			return 1;
		}
		else
		{
			printf("select() is successful!\n");
		}

		// #4 Check for arriving connections on the listening socket
		if (FD_ISSET(listenSocket, &ReadSet))
		{
			total--;
			acceptSocket = accept(listenSocket, NULL, NULL);
			if (acceptSocket == INVALID_SOCKET)
			{
				printf("accept() failed with error %d\n", WSAGetLastError());
				return 1;
			}
			else
			{
				iResult = ioctlsocket(acceptSocket, FIONBIO, &NonBlock);
				if (iResult == SOCKET_ERROR)
				{
					printf("ioctsocket() failed with error %d\n", WSAGetLastError());
				}
				else
				{
					printf("ioctlsocket() success!\n");

					ClientInfo* info = new ClientInfo();
					info->socket = acceptSocket;
					info->bytesRECV = 0;
					ClientArray[TotalClients] = info;
					TotalClients++;
					printf("New client connected on socket %d\n", (int)acceptSocket);
				}
			}
		}

		// #5 recv & send
		for (int i = 0; i < TotalClients; i++)
		{
			ClientInfo* client = ClientArray[i];

			// If the ReadSet is marked for this socket, then this means data
			// is available to be read on the socket
			if (FD_ISSET(client->socket, &ReadSet))
			{				
				total--;
				client->dataBuf.buf = client->buffer;
				client->dataBuf.len = DEFAULT_BUFLEN;

				DWORD Flags = 0;
				iResult = WSARecv(client->socket, &(client->dataBuf), 1, &RecvBytes, &Flags, NULL, NULL);

				if (iResult == SOCKET_ERROR)
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						// We can ignore this, it isn't an actual error.
					}
					else
					{
						printf("WSARecv failed on socket %d with error: %d\n", (int)client->socket, WSAGetLastError());
						RemoveClient(i);
					}
				}
				else
				{
					printf("WSARecv() is OK!\n");
					if (RecvBytes == 0)
					{
						RemoveClient(i);
					}
					else if (RecvBytes == SOCKET_ERROR)
					{
						printf("recv: There was an error..%d\n", WSAGetLastError());
						continue;
					}
					else
					{
						std::cout << recvMessage(client->dataBuf.buf, RecvBytes) << std::endl;

						//for (int i = 0; i < ReadSet.fd_count; i++)
						//{
						//	SOCKET BroacastSocket = ReadSet.fd_array[i];
						//	if (BroacastSocket != listenSocket && BroacastSocket != client->socket)
						//	{
						//		iResult = WSASend(client->socket, &(client->dataBuf), 1, &RecvBytes, Flags, NULL, NULL);
						//	}
						//}
						// RecvBytes > 0, we got data
						iResult = WSASend(client->socket, &(client->dataBuf), 1, &RecvBytes, Flags,	NULL, NULL);

						//int iSendResult = send(client->socket, client->dataBuf.buf, iResult, 0);

						if (iResult == SOCKET_ERROR)
						{
							printf("send error %d\n", WSAGetLastError());
						}
						else if (iResult == 0)
						{
							printf("Send result is 0\n");
						}
						else
						{
							printf("Successfully sent %d bytes!\n", iResult);
						}
					}
				}
			}
		}
	}


	// Create the master file descriptor set and zero it
	//fd_set master;
	//FD_ZERO(&master);

	// Add our first socket that we're interested in interacting with; the listening socket!
	// It's important that this socket is added for our server or else we won't 'hear' incoming
	// connections 
	//FD_SET(listenSocket, &master);

	//bool running = true;

	//while (running)
	//{
	//	fd_set copy = master;

	//	// See who's talking to us
	//	int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

	//	for (int i = 0; i < socketCount; i++)
	//	{
	//		// Makes things easy for us doing this assignment
	//		SOCKET sock = copy.fd_array[i];

	//		// Is it an inbound communication?
	//		if (sock == listenSocket)
	//		{
	//			// Accept a new connection
	//			SOCKET client = accept(listenSocket, nullptr, nullptr);

	//			// Add the new connection to the list of connected clients
	//			FD_SET(client, &master);

	//			printf("Accepted client on socket %d\n", client);
	//			// Send a welcome message to the connected client
	//			//std::string welcomeMsg = "Welcome to the Awesome Chat Server!\r\n";
	//			//send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
	//		}
	//		else // It's an inbound message
	//		{
	//			// Receive message
	//			int bytesReceived = recv(sock, recvBytes, recvBytesLen, 0);
	//			std::cout << recvMessage(recvBytes, bytesReceived) << std::endl;

	//			if (bytesReceived <= 0)
	//			{
	//				// Drop the client
	//				closesocket(sock);
	//				FD_CLR(sock, &master);
	//			}
	//			else
	//			{
	//				// Check to see if it's a command. \quit kills the server
	//				if (recvBytes[0] == '\\')
	//				{
	//					// Is the command quit? 
	//					std::string cmd = std::string(recvBytes, bytesReceived);
	//					if (cmd == "\\quit")
	//					{
	//						running = false;
	//						break;
	//					}

	//					// Unknown command
	//					continue;
	//				}

	//				// Send message to other clients, and definiately NOT the listening socket

	//				for (int i = 0; i < master.fd_count; i++)
	//				{
	//					SOCKET outSock = master.fd_array[i];
	//					if (outSock != listenSocket && outSock != sock)
	//					{
	//						//std::ostringstream ss;
	//						//ss << "SOCKET #" << sock << ": " << buf << "\r\n";
	//						//string strOut = ss.str();

	//						send(outSock, recvBytes, bytesReceived, 0);
	//					}
	//				}
	//			}
	//		}
	//	}

	//}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	//FD_CLR(listenSocket, &master);
	//closesocket(listenSocket);


	// #6 close
	iResult = shutdown(acceptSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(acceptSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	 closesocket(acceptSocket);
	WSACleanup();
	system("Pause");
	return 0;
}