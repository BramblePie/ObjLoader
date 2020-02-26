#include <iostream>

#include "objFileLoader.hpp"

int main()
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

	return 0;
}