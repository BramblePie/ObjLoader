#include "objectLoader.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>

struct Position
{
	float x, y, z;
	bool operator==(const Position& other)
	{
		return x == other.x && y == other.y && z == other.z;
	}
};

struct UV
{
	float x, y;
	bool operator==(const UV& other)
	{
		return x == other.x && y == other.y;
	}
};

struct Vertex
{
	Position* position = nullptr;
	UV* uv = nullptr;

	bool operator==(const Vertex& other)
	{
		return position == other.position && uv == other.uv;
	}

	bool operator!=(const Vertex& other)
	{
		return position != other.position || uv != other.uv;
	}
};

struct FaceElement { int v = -1, vt = -1; };

void parseFile(const char* path, std::vector<Position>& positions, std::vector<UV>& uvs, std::vector<FaceElement>& elements);

float* objectLoader(const char* path, unsigned int& bufferSize,
					unsigned int* (&indexBuffer), unsigned int& indexCount,
					unsigned int& positionCount,
					unsigned int& uvCount)
{
	std::vector<Position> positions;
	std::vector<UV> uvs;
	std::vector<FaceElement> elements;

	parseFile(path, positions, uvs, elements);

	positionCount = positions.size() > 0 ? 3 : 0;
	uvCount = uvs.size() > 0 ? 2 : 0;

	indexCount = elements.size();
	if (indexCount <= 0)
		return nullptr;
	indexBuffer = new unsigned int[indexCount]();
	std::vector<Vertex> vertices;

	// Split face elements into index buffer and vertex buffer
	unsigned int i = 0, ii = 0;
	for (auto e = elements.cbegin(); e != elements.cend(); e++, i++)
	{
		Vertex v;
		v.position = &positions[(*e).v];
		v.uv = uvCount ? &uvs[(*e).vt] : nullptr;

		auto it = std::find(vertices.begin(), vertices.end(), v);
		if (it != vertices.end())
		{
			// Add duplicate vertex index to index buffer
			indexBuffer[i] = std::distance(vertices.begin(), it);
		}
		else
		{
			// Non duplicate vertex at end of vertex buffer
			vertices.push_back(v);
			// Add new index to index buffer
			indexBuffer[i] = ii++;
		}
	}

	unsigned int step = positionCount + uvCount;
	bufferSize = step * vertices.size();
	float* buffer = new float[bufferSize];
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		buffer[step * i] = vertices[i].position->x;
		buffer[step * i + 1] = vertices[i].position->y;
		buffer[step * i + 2] = vertices[i].position->z;
		if (uvCount > 0)
		{
			buffer[step * i + 3] = vertices[i].uv->x;
			buffer[step * i + 4] = vertices[i].uv->y;
		}
	}

	bufferSize *= sizeof(float);	// Buffer size in bytes
	return buffer;
}

bool CheckOutOfBounds(int count, std::initializer_list<int> indices)
{
	bool b = false;
	for (auto&& i : indices)
		b |= i > count || i < 0;
	return b;
}

void parseFile(const char* path, std::vector<Position>& positions, std::vector<UV>& uvs, std::vector<FaceElement>& elements)
{
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
				Position tmp;
				if (3 == sscanf_s(cline, "v %f %f %f", &tmp.x, &tmp.y, &tmp.z))
					positions.push_back(tmp);
			}
			else if (0 == strcmp(head, "vt"))
			{
				hasUVs = true;
				parsedLinesCount++;
				UV tmp;
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

					elements.push_back({ pos_i[0] - 1, uv_i[0] - 1 });
					elements.push_back({ pos_i[1] - 1, uv_i[1] - 1 });
					elements.push_back({ pos_i[2] - 1, uv_i[2] - 1 });
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

					elements.push_back({ pos_i[0] - 1, uv_i[0] - 1 });
					elements.push_back({ pos_i[1] - 1, uv_i[1] - 1 });
					elements.push_back({ pos_i[2] - 1, uv_i[2] - 1 });
				}
				else if (!hasNormals && !hasUVs)
				{
					int pos_i[3]{};
					if (3 != sscanf_s(cline, "f %u %u %u", &pos_i[0], &pos_i[1], &pos_i[2]))
						continue;
					if (CheckOutOfBounds(positions.size(), { pos_i[0], pos_i[1], pos_i[2] }))
						continue;

					elements.push_back({ pos_i[0] - 1,  -1 });
					elements.push_back({ pos_i[1] - 1,  -1 });
					elements.push_back({ pos_i[2] - 1,  -1 });
				}
			}
		}
		file.close();
	}
};