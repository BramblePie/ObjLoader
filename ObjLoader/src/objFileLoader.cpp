#include "objFileLoader.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

struct Position { float x, y, z; };

struct Normal { float x, y, z; };

struct UV { float x, y; };

struct Vertex
{
	Position* position;
	Normal* normal;
	UV* uv;
};

unsigned int g_count = 0, g_position_size = 0, g_normal_size = 0, g_uv_size = 0;

std::queue<Vertex> vertices;
std::vector<Position> pos;
std::vector<Normal> normals;
std::vector<UV> uvs;

void parse_face(std::string line);

float* make_out_array(unsigned int count);

Normal* generate_normals(Vertex face[3]);

float* loadObject(const char* path, unsigned int& count, unsigned int& position_size, unsigned int& normal_size, unsigned int& uv_size)
{
	// count			=		number of vertices
	// position_size	=		byte size of a position
	// normal_size		=		byte size of a normal
	// uv_size			=		byte size of a UV

	// Resulting vertices
	float* out_vertices = nullptr;

	std::ifstream file;
	file.open(path);
	if (file.is_open())
	{
		const unsigned int n = 128;
		unsigned int count_pos = 0, count_norm = 0, count_uv = 0;
		while (!file.eof())
		{
			std::string line;
			std::getline(file, line);

			char head[n]{};
			int res = sscanf_s(line.c_str(), "%s", head, n);
			if (0 == strcmp(head, "#") || res == 0)
			{
				// Comment or blank line, go next
			}
			else if (0 == strcmp(head, "v"))
			{		// Vertex position found
				count_pos++;
				Position tmp;
				if (4 == sscanf_s(line.c_str(), "%s%f %f %f", head, n, &tmp.x, &tmp.y, &tmp.z))
					pos.push_back(tmp);
			}
			else if (0 == strcmp(head, "vn"))
			{		// Vertex normal found
				count_norm++;
				Normal tmp;
				if (4 == sscanf_s(line.c_str(), "%s%f %f %f", head, n, &tmp.x, &tmp.y, &tmp.z))
					normals.push_back(tmp);
			}
			else if (0 == strcmp(head, "vt"))
			{		// Vertex uv found
				count_uv++;
				UV tmp;
				if (3 == sscanf_s(line.c_str(), "%s%f %f", head, n, &tmp.x, &tmp.y))
					uvs.push_back(tmp);
			}
			else if (0 == strcmp(head, "f"))
			{		// Face found
				// Check stride for position, normal and uv
				if (!(g_position_size | g_normal_size | g_uv_size))
				{
					if (count_pos != pos.size() || count_norm != normals.size() || count_uv != uvs.size())
						break;
					g_position_size = sizeof(pos[0]);
					g_normal_size = normals.size() > 0 ? sizeof(normals[0]) : 0;
					g_uv_size = uvs.size() > 0 ? sizeof(uvs[0]) : 0;
				}
				parse_face(line);
			}
		}
	}
	else
	{
		std::cout << "ERROR :: File \"" << path << "\" NOT FOUND or NO ACCESS" << std::endl;
		return nullptr;
	}

	position_size = g_position_size;
	normal_size = g_position_size;
	uv_size = g_uv_size;
	count = vertices.size();

	if (count)
		return make_out_array(count);
	return nullptr;
}

float* make_out_array(unsigned int count)
{
	// Stride in floats
	// Normals are generated if not present
	unsigned int stride = g_position_size / sizeof(float) + g_position_size / sizeof(float) + g_uv_size / sizeof(float);

	float* out = new float[count * stride];
	unsigned int i = 0;
	if (stride == 8)
	{
		for (Vertex v; !vertices.empty(); vertices.pop(), i += stride)
		{
			v = vertices.front();
			// Out position
			out[i] = v.position->x;
			out[i + 1] = v.position->y;
			out[i + 2] = v.position->z;
			// Out normal
			out[i + 3] = v.normal->x;
			out[i + 4] = v.normal->y;
			out[i + 5] = v.normal->z;
			// Out UV
			out[i + 6] = v.uv->x;
			out[i + 7] = v.uv->y;
		}
	}
	else if (stride == 6)
	{
		for (Vertex v; !vertices.empty(); vertices.pop(), i += stride)
		{
			v = vertices.front();
			// Out position
			out[i] = v.position->x;
			out[i + 1] = v.position->y;
			out[i + 2] = v.position->z;
			// Out normal
			out[i + 3] = v.normal->x;
			out[i + 4] = v.normal->y;
			out[i + 5] = v.normal->z;
		}
	}
	return out;
}

bool CheckOutOfBounds(unsigned int count, std::initializer_list<unsigned int> indices);

void parse_face(std::string line)
{
	unsigned int pos_i[3]{}, uv_i[3]{}, norm_i[3]{};
	if (g_position_size > 0 && g_normal_size > 0 && g_uv_size > 0)
	{
		if (9 != sscanf_s(line.c_str(), "f %u/%u/%u %u/%u/%u %u/%u/%u",
						  &pos_i[0], &uv_i[0], &norm_i[0],
						  &pos_i[1], &uv_i[1], &norm_i[1],
						  &pos_i[2], &uv_i[2], &norm_i[2]))
			return;
		if (CheckOutOfBounds(pos.size(), { pos_i[0], pos_i[1], pos_i[2] }))
			return;
		if (CheckOutOfBounds(uvs.size(), { uv_i[0], uv_i[1], uv_i[2] }))
			return;
		if (CheckOutOfBounds(normals.size(), { norm_i[0], norm_i[1], norm_i[2] }))
			return;

		Vertex face[3]{};
		face[0].position = &pos[pos_i[0] - 1];
		face[1].position = &pos[pos_i[1] - 1];
		face[2].position = &pos[pos_i[2] - 1];

		face[0].uv = &uvs[uv_i[0] - 1];
		face[1].uv = &uvs[uv_i[1] - 1];
		face[2].uv = &uvs[uv_i[2] - 1];

		face[0].normal = &normals[norm_i[0] - 1];
		face[1].normal = &normals[norm_i[1] - 1];
		face[2].normal = &normals[norm_i[2] - 1];

		vertices.push(face[0]);
		vertices.push(face[1]);
		vertices.push(face[2]);
	}
	else if (g_position_size > 0 && g_uv_size > 0)
	{
		if (6 != sscanf_s(line.c_str(), "f %u/%u %u/%u %u/%u",
						  &pos_i[0], &uv_i[0],
						  &pos_i[1], &uv_i[1],
						  &pos_i[2], &uv_i[2]))
			return;
		if (CheckOutOfBounds(pos.size(), { pos_i[0], pos_i[1], pos_i[2] }))
			return;
		if (CheckOutOfBounds(uvs.size(), { uv_i[0], uv_i[1], uv_i[2] }))
			return;

		Vertex face[3]{};
		face[0].position = &pos[pos_i[0] - 1];
		face[1].position = &pos[pos_i[1] - 1];
		face[2].position = &pos[pos_i[2] - 1];

		face[0].uv = &uvs[uv_i[0] - 1];
		face[1].uv = &uvs[uv_i[1] - 1];
		face[2].uv = &uvs[uv_i[2] - 1];

		Normal* n = generate_normals(face);
		face[0].normal = n;
		face[1].normal = n;
		face[2].normal = n;

		vertices.push(face[0]);
		vertices.push(face[1]);
		vertices.push(face[2]);
	}
	else if (g_position_size > 0)
	{
		if (3 != sscanf_s(line.c_str(), "f %u %u %u", &pos_i[0], &pos_i[1], &pos_i[2]))
			return;
		if (CheckOutOfBounds(pos.size(), { pos_i[0], pos_i[1], pos_i[2] }))
			return;

		Vertex face[3]{};
		face[0].position = &pos[pos_i[0] - 1];
		face[1].position = &pos[pos_i[1] - 1];
		face[2].position = &pos[pos_i[2] - 1];

		Normal* n = generate_normals(face);
		face[0].normal = n;
		face[1].normal = n;
		face[2].normal = n;

		vertices.push(face[0]);
		vertices.push(face[1]);
		vertices.push(face[2]);
	}
}

bool CheckOutOfBounds(unsigned int count, std::initializer_list<unsigned int> indices)
{
	bool b = false;
	for (auto&& i : indices)
		b |= i > count;
	return b;
}

Normal* generate_normals(Vertex face[3])
{
	glm::vec3 a = glm::vec3(face[0].position->x, face[0].position->y, face[0].position->z);
	// b and c relative to a
	glm::vec3 b = glm::vec3(face[1].position->x, face[1].position->y, face[1].position->z) - a;
	glm::vec3 c = glm::vec3(face[2].position->x, face[2].position->y, face[2].position->z) - a;

	glm::vec3 n = glm::normalize(glm::cross(b, c));
	Normal* res = new Normal();
	res->x = n.x;
	res->y = n.y;
	res->z = n.z;
	return res;
}