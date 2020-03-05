#include "objFileLoader.h"

#include <glm/glm.hpp>
#include <vector>
#include <fstream>
#include <string>

struct Face
{
	glm::vec3* positions[3]{};
	glm::vec2* uvs[3]{};
	glm::vec3* normals[3]{};
	bool isIndexed[3]{};
};

std::vector<Face> parseFile(const char* path,
							std::vector<glm::vec3>& positions,
							std::vector<glm::vec2>& uvs,
							std::vector<glm::vec3>& normals);
template <typename T>
int getIndexVector(std::vector<T> v, T value);

float* loadObject(const char* path, unsigned int& bufferSize,
				  unsigned int* (&indexBuffer), unsigned int& indexCount,
				  unsigned int& positionCount, unsigned int& normalCount, unsigned int& uvCount)
{
	bool isBad = false;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	std::vector<Face> faces = parseFile(path, positions, uvs, normals);

	positionCount = positions.size() > 0 ? 3 : 0;
	normalCount = normals.size() > 0 ? 3 : 0;
	//uvCount = uvs.size() > 0 ? 2 : 0;
	uvCount = 0;

	indexCount = faces.size() * 3;
	indexBuffer = new unsigned int[indexCount];

	unsigned int ii = 0;
	for (Face f : faces)
	{
		// Calculate normals
		glm::vec3 a = *f.positions[1] - *f.positions[0];
		glm::vec3 b = *f.positions[2] - *f.positions[0];
		glm::vec3 n = glm::normalize(glm::cross(a, b));
		*f.normals[0] += n;
		*f.normals[1] += n;
		*f.normals[2] += n;
		// Sum of normals is normalised later

		// Fill index buffer
		indexBuffer[ii++] = getIndexVector(positions, *f.positions[0]);
		indexBuffer[ii++] = getIndexVector(positions, *f.positions[1]);
		indexBuffer[ii++] = getIndexVector(positions, *f.positions[2]);
	}

	unsigned int step = positionCount + normalCount;
	bufferSize = positions.size() * step;
	float* vertexBuffer = new float[bufferSize];
	// Fill vertex buffer
	for (unsigned int i = 0; i < positions.size(); i++)
	{
		normals[i] = glm::normalize(normals[i]);

		vertexBuffer[i * step] = positions[i].x;
		vertexBuffer[i * step + 1] = positions[i].y;
		vertexBuffer[i * step + 2] = positions[i].z;
		vertexBuffer[i * step + 3] = normals[i].x;
		vertexBuffer[i * step + 4] = normals[i].y;
		vertexBuffer[i * step + 5] = normals[i].z;
	}

	bufferSize *= sizeof(float);	// Buffer size in bytes
	return vertexBuffer;
}

bool CheckOutOfBounds(int count, std::initializer_list<int> indices)
{
	bool b = false;
	for (auto&& i : indices)
		b |= i > count || i <= 0;
	return b;
}

std::vector<Face> parseFile(const char* path,
							std::vector<glm::vec3>& positions,
							std::vector<glm::vec2>& uvs,
							std::vector<glm::vec3>& normals)
{
	std::vector<Face> faces;

	std::ifstream file;
	file.open(path);
	if (file.is_open())
	{
		const unsigned int n = 128;
		unsigned int parsedLinesCount = 0;
		bool hasNormals = false, hasUVs = false;

		while (!file.eof())
		{
			std::string line;
			std::getline(file, line);
			const char* cline = line.c_str();

			char head[n]{};
			int found = sscanf_s(cline, "%s", head, n);

			if (0 == strcmp(head, "v"))
			{
				parsedLinesCount++;
				glm::vec3 tmp;
				if (3 == sscanf_s(cline, "v %f %f %f", &tmp.x, &tmp.y, &tmp.z))
				{
					positions.push_back(tmp);
					normals.push_back(glm::vec3(0.0f));
				}
			}
			else if (0 == strcmp(head, "vt"))
			{
				hasUVs = true;
				parsedLinesCount++;
				glm::vec3 tmp;
				if (2 == sscanf_s(cline, "vt %f %f", &tmp.x, &tmp.y))
					uvs.push_back(tmp);
			}
			else if (0 == strcmp(head, "vn"))
			{
				hasNormals = true;
			}
			else if (0 == strcmp(head, "f"))
			{
				if (hasNormals && hasUVs)
				{
					int pos_i[3]{}, uv_i[3]{}, norm_i[3]{};
					if (9 != sscanf_s(cline, "f %d/%d/%d %d/%d/%d %d/%d/%d",
									  &pos_i[0], &uv_i[0], &norm_i[0],
									  &pos_i[1], &uv_i[1], &norm_i[1],
									  &pos_i[2], &uv_i[2], &norm_i[2]))
						continue;
					if (CheckOutOfBounds(positions.size(), { pos_i[0], pos_i[1], pos_i[2] }))
						continue;
					if (CheckOutOfBounds(uvs.size(), { uv_i[0], uv_i[1], uv_i[2] }))
						continue;

					// Make face
					Face tmp;
					tmp.positions[0] = &positions[pos_i[0] - 1];
					tmp.positions[1] = &positions[pos_i[1] - 1];
					tmp.positions[2] = &positions[pos_i[2] - 1];
					// Link normal pointers
					tmp.normals[0] = &normals[pos_i[0] - 1];
					tmp.normals[1] = &normals[pos_i[1] - 1];
					tmp.normals[2] = &normals[pos_i[2] - 1];
					faces.push_back(tmp);
				}
				else if (hasUVs && !hasNormals)
				{
					int pos_i[3]{}, uv_i[3]{};
					if (6 != sscanf_s(cline, "f %u/%u %u/%u %u/%u",
									  &pos_i[0], &uv_i[0],
									  &pos_i[1], &uv_i[1],
									  &pos_i[2], &uv_i[2]))
						continue;
					if (CheckOutOfBounds(positions.size(), { pos_i[0], pos_i[1], pos_i[2] }))
						continue;
					if (CheckOutOfBounds(uvs.size(), { uv_i[0], uv_i[1], uv_i[2] }))
						continue;

					// Make face
					Face tmp;
					tmp.positions[0] = &positions[pos_i[0] - 1];
					tmp.positions[1] = &positions[pos_i[1] - 1];
					tmp.positions[2] = &positions[pos_i[2] - 1];
					// Link normal pointers
					tmp.normals[0] = &normals[pos_i[0] - 1];
					tmp.normals[1] = &normals[pos_i[1] - 1];
					tmp.normals[2] = &normals[pos_i[2] - 1];
					faces.push_back(tmp);
				}
				else if (!hasNormals && !hasUVs)
				{
					int pos_i[3]{};
					if (3 != sscanf_s(cline, "f %u %u %u", &pos_i[0], &pos_i[1], &pos_i[2]))
						continue;
					if (CheckOutOfBounds(positions.size(), { pos_i[0], pos_i[1], pos_i[2] }))
						continue;
					Face tmp;
					tmp.positions[0] = &positions[pos_i[0] - 1];
					tmp.positions[1] = &positions[pos_i[1] - 1];
					tmp.positions[2] = &positions[pos_i[2] - 1];
					// Link normal pointers
					tmp.normals[0] = &normals[pos_i[0] - 1];
					tmp.normals[1] = &normals[pos_i[1] - 1];
					tmp.normals[2] = &normals[pos_i[2] - 1];
					faces.push_back(tmp);
				}
			}
		}
		file.close();
	}

	return faces;
}

template<typename T>
int getIndexVector(std::vector<T> v, T value)
{
	auto it = std::find(v.begin(), v.end(), value);
	if (it != v.end())
		return std::distance(v.begin(), it);
	return -1;
}