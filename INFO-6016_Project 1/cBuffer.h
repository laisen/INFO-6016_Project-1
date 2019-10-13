#pragma once

#include <vector>
#include <string>

class cBuffer
{
public:
	size_t size;
	std::vector<uint8_t> _buffer;
	
	cBuffer();
	~cBuffer();	

	void writeInt32BE(std::size_t index, int32_t value);

	void writeShortBE(std::size_t index, short value);

	void writeStringBE(std::size_t index, std::string value);


	uint32_t readUInt32BE(std::size_t index);

	short readShortBE(std::size_t index);

	std::string readStringBE(std::size_t index, int strlen);
};