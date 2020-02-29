#pragma once

float* loadObject(const char* path,
				  unsigned int& count,
				  unsigned int& position_size,
				  unsigned int& normal_size,
				  unsigned int& uv_size);

float* loadObjectByIndex(const char* path,
						 unsigned int& vertex_count,
						 unsigned int& position_size,
						 unsigned int& normal_size,
						 unsigned int& uv_size,
						 unsigned int* indices,
						 unsigned int& index_count);