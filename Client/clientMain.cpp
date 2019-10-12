#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "cBuffer.h"
#include "cProtocol.h"

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "5150"

int main(int argc, char** argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	const char* sendbuf = "this is a test";
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Validate the parameters
	//if (argc != 2) {
	//	printf("usage: %s server-name\n", argv[0]);
	//	return 1;
	//}

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

	// Send an initial buffer
	//printf("Sending a packet to the server...\n");
	//system("Pause");
	//iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
	//if (iResult == SOCKET_ERROR) {
	//	printf("send failed with error: %d\n", WSAGetLastError());
	//	closesocket(ConnectSocket);
	//	WSACleanup();
	//	return 1;
	//}

	//printf("Bytes Sent: %ld\n", iResult);

	char buf[4096];
	std::string userInput;

	std::string str;
	cBuffer Buffer;
	Buffer = JoinRoom("room#1", "Sen");
	str.insert(str.begin(), Buffer._buffer.begin(), Buffer._buffer.end());
	std::cout << Buffer.readUInt32BE(0) << std::endl;
	std::cout << Buffer.readUInt32BE(4) << std::endl;
	std::cout << Buffer.readUInt32BE(8) << std::endl;
	std::cout << Buffer.readUInt32BE(18) << std::endl;
	iResult = send(ConnectSocket, str.c_str(), str.size(), 0);
	do
	{
		// Prompt the user for some text
		std::cout << "> ";
		getline(std::cin, userInput);

		if (userInput.size() > 0)		// Make sure the user has typed in something
		{
			//cBuffer sendBuffer;
			//sendBuffer.writeStringBE(0, userInput);
			//std::string bufferToString;
			//bufferToString.insert(bufferToString.begin(), sendBuffer._buffer.begin(), sendBuffer._buffer.end());

			// Send the text
			int sendResult = send(ConnectSocket, userInput.c_str(), userInput.size() + 1, 0);
			//int sendResult = send(ConnectSocket, bufferToString.c_str(), bufferToString.size(), 0);
			if (sendResult != SOCKET_ERROR)
			{
				// Wait for response
				ZeroMemory(buf, 4096);
				int bytesReceived = recv(ConnectSocket, buf, 4096, 0);
				if (bytesReceived > 0)
				{
					//cBuffer recvBuffer;
					//recvBuffer.writeStringBE(0, std::string(buf, 0, bytesReceived));
					// Echo response to console
					std::cout << "SERVER> " << /*recvBuffer.readStringBE(0) << */std::endl;
				}
			}
		}

	} while (userInput.size() > 0);

	// shutdown the connection since no more data will be sent
	//iResult = shutdown(ConnectSocket, SD_SEND);
	//if (iResult == SOCKET_ERROR) {
	//	printf("shutdown failed with error: %d\n", WSAGetLastError());
	//	closesocket(ConnectSocket);
	//	WSACleanup();
	//	return 1;
	//}
	//printf("Successfully shutdown socket %d!\n", (int)ConnectSocket);

	// Receive until the peer closes the connection
	//do {
	//	printf("Waiting to receive data from the server...\n");
	//	system("Pause");
	//	iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
	//	if (iResult > 0)
	//		printf("Bytes received: %d\n", iResult);
	//	else if (iResult == 0)
	//		printf("Connection closed\n");
	//	else
	//		printf("recv failed with error: %d\n", WSAGetLastError());

	//} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}