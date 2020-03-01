#include <iostream>

#include "objFileLoader.hpp"

#include "objectLoader.h"

void testObjFileLoader()
{
	unsigned int count, position_size, normal_size, uv_size;
	float* buffer = loadObject(R"(src\pbox.obj)", count, position_size, normal_size, uv_size);

	unsigned int p = position_size / sizeof(float), n = normal_size / sizeof(float), u = uv_size / sizeof(float);
	unsigned int s = p + n + u;
	for (unsigned int i = 0; i < count && buffer; i++)
	{
		std::cout << buffer[s * i] << ", " << buffer[s * i + 1] << ", " << buffer[s * i + 2] << std::endl;
		std::cout << "  " << buffer[s * i + p] << ", " << buffer[s * i + p + 1] << ", " << buffer[s * i + p + 2] << std::endl;
		if (uv_size)
			std::cout << "    " << buffer[s * i + p + n] << ", " << buffer[s * i + p + n + 1] << std::endl;
	}
}

int main()
{
	unsigned int* ib;
	unsigned int bufferSize, indexCount, posCount, uvCount;
	float* vb = objectLoader(R"(src\pbox.obj)", bufferSize, ib, indexCount, posCount, uvCount);

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