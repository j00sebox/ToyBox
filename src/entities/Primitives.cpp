#include "pch.h"
#include "Primitives.h"

#include "Buffer.h"

#include <cstring>

std::string primitve_type_to_str(PrimitiveTypes pt)
{
	switch (pt)
	{
	case PrimitiveTypes::None:
		return "none";
	case PrimitiveTypes::Cube:
		return "cube";
	case PrimitiveTypes::Quad:
		return "quad";
	default:
		break;
	}
}

PrimitiveTypes str_to_primitive_type(const char* name)
{
	if (!strcmp(name, "cube"))
	{
		return PrimitiveTypes::Cube;
	}
	else if (!strcmp(name, "quad"))
	{
		return PrimitiveTypes::Quad;
	}

	return PrimitiveTypes::None;
}

const std::vector<float> Cube::vertices = {
	-1.0f, -1.0f,  1.0f, 0.f, 0.f, -1.f, 0.f, -1.f,		//        7--------6
	 1.0f, -1.0f,  1.0f, 0.f, 0.f, -1.f, 0.f, -1.f,		//       /|       /|
	 1.0f, -1.0f, -1.0f, 0.f, 0.f,  1.f, 0.f,  1.f,		//      4--------5 |
	-1.0f, -1.0f, -1.0f, 0.f, 0.f,  1.f, 0.f,  1.f,		//      | |      | |
	-1.0f,  1.0f,  1.0f, 0.f, 0.f, -1.f, 0.f, -1.f,		//      | 3------|-2
	 1.0f,  1.0f,  1.0f, 0.f, 0.f, -1.f, 0.f, -1.f,		//      |/       |/
	 1.0f,  1.0f, -1.0f, 0.f, 0.f,  1.f, 0.f,  1.f,		//      0--------1
	-1.0f,  1.0f, -1.0f, 0.f, 0.f,  1.f, 0.f,  1.f
};

const std::vector<unsigned int> Cube::indices = {
	// right
	1, 2, 6,
	6, 5, 1,

	// left
	0, 4, 7,
	7, 3, 0,

	// top
	4, 5, 6,
	6, 7, 4,

	// bottom
	0, 3, 2,
	2, 1, 0,

	// back
	0, 1, 5,
	5, 4, 0,

	// front
	3, 7, 6,
	6, 2, 3
};

const std::vector<float> Quad::vertices = {
		-0.5f, -0.5f, -0.5f, 0.f, 0.f, 1.f, 0.f, 0.f,
		-0.5f,	0.5f, -0.5f, 0.f, 0.f, 1.f, 0.f, 1.f,
		 0.5f,  0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 1.f,
		 0.5f, -0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 0.f
};

const std::vector<unsigned int> Quad::indices = {
		0, 2, 1,
		0, 3, 2
};