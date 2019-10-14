#pragma once

#include <string>
#include "cBuffer.h"
#include <sstream>

const int MESSAGE_ID_JOIN_ROOM = 1;
const int MESSAGE_ID_SEND = 2;
const int MESSAGE_ID_LEAVE_ROOM = 3;
int packet_length;
int room_length;
int message_length;
int client_length;
int message_type;

std::string joinRoom(std::string roomName, std::string clientName)
{
	cBuffer buffer;

	// header
	packet_length = sizeof(packet_length)
		+ sizeof(MESSAGE_ID_JOIN_ROOM)
		+ sizeof(room_length)
		+ roomName.length()
		+ sizeof(client_length)
		+ clientName.length();
	buffer.writeInt32BE(0, packet_length);

	buffer.writeInt32BE(sizeof(packet_length), MESSAGE_ID_JOIN_ROOM);

	// room
	buffer.writeInt32BE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_JOIN_ROOM),
		roomName.length());

	buffer.writeStringBE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_JOIN_ROOM)
		+ sizeof(room_length),
		roomName);

	// client
	buffer.writeInt32BE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_JOIN_ROOM)
		+ sizeof(room_length)
		+ roomName.length(),
		clientName.length());

	buffer.writeStringBE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_JOIN_ROOM)
		+ sizeof(room_length)
		+ roomName.length()
		+ sizeof(client_length),
		clientName);

	std::string bufferToString;
	bufferToString.insert(bufferToString.begin(), buffer._buffer.begin(), buffer._buffer.end());

	return bufferToString;
}

std::string sendMessage(std::string roomName, std::string clientName, std::string message)
{
	cBuffer buffer;

	// header
	packet_length = sizeof(packet_length)
		+ sizeof(MESSAGE_ID_JOIN_ROOM)
		+ sizeof(room_length)
		+ roomName.length()
		+ sizeof(client_length)
		+ clientName.length()
		+ sizeof(message_length)
		+ message.length();
	buffer.writeInt32BE(0, packet_length);

	buffer.writeInt32BE(sizeof(packet_length), MESSAGE_ID_SEND);

	// room
	buffer.writeInt32BE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_SEND),
		roomName.length());

	buffer.writeStringBE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_SEND)
		+ sizeof(room_length),
		roomName);

	// client
	buffer.writeInt32BE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_SEND)
		+ sizeof(room_length)
		+ roomName.length(),
		clientName.length());

	buffer.writeStringBE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_SEND)
		+ sizeof(room_length)
		+ roomName.length()
		+ sizeof(client_length),
		clientName);

	// message
	buffer.writeInt32BE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_SEND)
		+ sizeof(room_length)
		+ roomName.length()
		+ sizeof(client_length)
		+ clientName.length(),
		message.length());

	buffer.writeStringBE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_SEND)
		+ sizeof(room_length)
		+ roomName.length()
		+ sizeof(client_length)
		+ clientName.length()
		+ sizeof(message_length),
		message);

	std::string bufferToString;
	bufferToString.insert(bufferToString.begin(), buffer._buffer.begin(), buffer._buffer.end());

	return bufferToString;
}

std::string leaveRoom(std::string roomName, std::string clientName)
{
	cBuffer buffer;

	// header
	packet_length = sizeof(packet_length)
		+ sizeof(MESSAGE_ID_LEAVE_ROOM)
		+ sizeof(room_length)
		+ roomName.length()
		+ sizeof(client_length)
		+ clientName.length();
	buffer.writeInt32BE(0, packet_length);

	buffer.writeInt32BE(sizeof(packet_length), MESSAGE_ID_LEAVE_ROOM);

	// room
	buffer.writeInt32BE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_LEAVE_ROOM),
		roomName.length());

	buffer.writeStringBE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_LEAVE_ROOM)
		+ sizeof(room_length),
		roomName);

	// client
	buffer.writeInt32BE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_LEAVE_ROOM)
		+ sizeof(room_length)
		+ roomName.length(),
		clientName.length());

	buffer.writeStringBE(sizeof(packet_length)
		+ sizeof(MESSAGE_ID_LEAVE_ROOM)
		+ sizeof(room_length)
		+ roomName.length()
		+ sizeof(client_length),
		clientName);

	std::string bufferToString;
	bufferToString.insert(bufferToString.begin(), buffer._buffer.begin(), buffer._buffer.end());

	return bufferToString;
}

std::string recvMessage(char recvBytes[], int bytesReceived)
{
	cBuffer buffer;	
	buffer._buffer.resize(bytesReceived);
	for (size_t i = 0; i < bytesReceived; i++)
	{
		buffer._buffer[i] = recvBytes[i];
	}
	
	packet_length = bytesReceived;

	message_type = buffer.readUInt32BE(sizeof(packet_length));

	room_length = buffer.readUInt32BE(sizeof(packet_length) + sizeof(message_type));

	client_length = buffer.readUInt32BE(sizeof(packet_length)
		+ sizeof(message_type)
		+ sizeof(room_length)
		+ room_length);

	std::string roomName = buffer.readStringBE(sizeof(packet_length)
		+ sizeof(message_type)
		+ sizeof(room_length), room_length);

	std::string clientName = buffer.readStringBE(sizeof(packet_length)
		+ sizeof(message_type)
		+ sizeof(room_length)
		+ room_length
		+ sizeof(client_length), client_length);

	if (message_type == MESSAGE_ID_JOIN_ROOM)
	{
		std::ostringstream outString;
		outString << "Welcome " << clientName << " joins " << roomName << "! (Input /leave to exit)";		

		return outString.str();
	}

	if (message_type == MESSAGE_ID_SEND)
	{
		message_length = buffer.readUInt32BE(sizeof(packet_length)
			+ sizeof(message_type)
			+ sizeof(room_length)
			+ room_length
			+ sizeof(client_length)
			+ client_length);

		std::string message = buffer.readStringBE(sizeof(packet_length)
			+ sizeof(message_type) 
			+ sizeof(room_length)
			+ room_length
			+ sizeof(client_length) 
			+ client_length
			+ sizeof(message_length), message_length);

		std::ostringstream outString;
		outString << "[" << roomName << "] [" << clientName << "]: " << message;

		return outString.str();
	}

	if (message_type == MESSAGE_ID_LEAVE_ROOM)
	{		
		std::ostringstream outString;
		outString << clientName << " has left " << roomName << ".";

		return outString.str();
	}

	return "unknown message";
}