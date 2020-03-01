#pragma once

float* objectLoader(const char* path, unsigned int& bufferSize,
					unsigned int* (&indexBuffer), unsigned int& indexCount,
					unsigned int& positionCount,
					unsigned int& uvCount);