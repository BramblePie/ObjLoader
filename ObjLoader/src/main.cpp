#include <iostream>

#include "objFileLoader.h"

int main()
{
	unsigned int* ib;
	unsigned int bufferSize, indexCount, posCount, uvCount;
	float* vb = loadObject(R"(src\pbox.obj)", bufferSize, ib, indexCount, posCount, uvCount);

	std::cout << "Position count: " << posCount << ", UV count: " << uvCount << std::endl;
	std::cout << "Buffer size(bytes): " << bufferSize << ", index count: " << indexCount << std::endl << std::endl;

	unsigned int step = posCount + uvCount;
	unsigned int n = bufferSize / (step * sizeof(float));
	for (unsigned int i = 0; i < n; i++)
	{
		for (unsigned int j = 0; j < step; j++)
			std::cout << vb[step * i + j] << ",\t";
		std::cout << std::endl;
	}

	for (unsigned int i = 0; i < indexCount; i++)
	{
		std::cout << ib[i] << ", ";
		if ((i + 1) % 3 == 0)
			std::cout << std::endl;
	}

	return 0;
}