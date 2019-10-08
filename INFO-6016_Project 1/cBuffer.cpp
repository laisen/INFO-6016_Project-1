#include "cBuffer.h"
#include <bitset>
#include <iostream>

cBuffer::cBuffer()
{
	size = 0;
}

cBuffer::~cBuffer()
{
}

void cBuffer::writeInt32BE(std::size_t index, int32_t value)
{
	_buffer.resize(_buffer.size() + 4);
	_buffer[index] = value >> 24;
	_buffer[index + 1] = value >> 16;
	_buffer[index + 2] = value >> 8;
	_buffer[index + 3] = value;

	return;
}

void cBuffer::writeShortBE(std::size_t index, short value)
{
	_buffer.resize(_buffer.size() + 2);
	_buffer[index] = value >> 8;
	_buffer[index + 1] = value;

	return;
}

void cBuffer::writeStringBE(std::size_t index, std::string value)
{
	_buffer.resize(_buffer.size() + value.length());
	for (int stringIndex = 0; stringIndex < value.length(); stringIndex++)
	{
		_buffer[index + stringIndex] = value[stringIndex];
		//printf("buffer %d write %s\n", stringIndex, std::bitset<8>(_buffer[stringIndex]).to_string().c_str());
	}

	return;
}

uint32_t cBuffer::readUInt32BE(std::size_t index)
{
	uint32_t value = 0;

	value |= _buffer[index] << 24;
	value |= _buffer[index + 1] << 16;
	value |= _buffer[index + 2] << 8;
	value |= _buffer[index + 3];

	return value;
}

short cBuffer::readShortBE(std::size_t index)
{
	unsigned short value = 0;

	value |= _buffer[index] << 8;
	value |= _buffer[index + 1];
	
	return value;
}

std::string cBuffer::readStringBE(std::size_t index)
{
	std::string value;
	value.resize(_buffer.size() - index + 1);
	for (int bufferIndex = 0; bufferIndex < _buffer.size() - index; bufferIndex++)
	{
		value[bufferIndex] = _buffer[index + bufferIndex];
		//printf("value %d read %s\n", bufferIndex, std::bitset<8>(value[bufferIndex]).to_string().c_str());
	}
	
	return value;
}