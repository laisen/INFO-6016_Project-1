#pragma once

#include <string>
#include "cBuffer.h"

const int MESSAGE_ID_JOIN_ROOM = 1;
const int MESSAGE_ID_SEND = 2;
const int MESSAGE_ID_LEAVE_ROOM = 3;
int PACKET_LENGTH;
int ROOM_LENGTH;
int MESSAGE_LENGTH;
int CLIENT_LENGTH;

cBuffer JoinRoom(std::string roomName, std::string clientName)
{
	cBuffer buffer;

	PACKET_LENGTH = sizeof(PACKET_LENGTH)
		+ sizeof(MESSAGE_ID_JOIN_ROOM)
		+ sizeof(ROOM_LENGTH)
		+ roomName.length()
		+ sizeof(CLIENT_LENGTH)
		+ clientName.length();
	buffer.writeInt32BE(0, PACKET_LENGTH);

	buffer.writeInt32BE(sizeof(PACKET_LENGTH), MESSAGE_ID_JOIN_ROOM);

	buffer.writeInt32BE(sizeof(PACKET_LENGTH)
		+ sizeof(MESSAGE_ID_JOIN_ROOM),
		roomName.length());

	buffer.writeStringBE(sizeof(PACKET_LENGTH)
		+ sizeof(MESSAGE_ID_JOIN_ROOM)
		+ sizeof(ROOM_LENGTH),
		roomName);

	buffer.writeInt32BE(sizeof(PACKET_LENGTH)
		+ sizeof(MESSAGE_ID_JOIN_ROOM)
		+ sizeof(ROOM_LENGTH)
		+ roomName.length(),
		clientName.length());

	buffer.writeStringBE(sizeof(PACKET_LENGTH)
		+ sizeof(MESSAGE_ID_JOIN_ROOM)
		+ sizeof(ROOM_LENGTH)
		+ roomName.length()
		+ sizeof(CLIENT_LENGTH),
		clientName);

	return buffer;
}
