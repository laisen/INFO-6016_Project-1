#include <iostream>
#include "cBuffer.h"
//#include <bitset>			// Required for bitset
//#include <string>

int main(int argc, char** argv)
{
	cBuffer MyBuffer;
	MyBuffer.writeInt32BE(0, 123);
	MyBuffer.writeStringBE(4, "Hello");
	MyBuffer.writeShortBE(strlen("Hello") + 4, 666);

	std::cout << MyBuffer.readUInt32BE(0) << std::endl;
	std::cout << MyBuffer.readStringBE(4) << std::endl;
	std::cout << MyBuffer.readShortBE(strlen("Hello") + 4) << std::endl;
	std::cout << sizeof(int) << std::endl;
	

	system("Pause");
	return 0;
}