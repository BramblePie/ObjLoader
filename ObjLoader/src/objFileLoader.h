#pragma once

float* loadObject(const char* path, unsigned int& bufferSize,
				  unsigned int* (&indexBuffer), unsigned int& indexCount,
				  unsigned int& positionCount,
				  unsigned int& normalCount,
				  unsigned int& uvCount);