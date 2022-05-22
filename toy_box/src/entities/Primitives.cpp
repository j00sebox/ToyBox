#include "pch.h"
#include "Primitives.h"

#include "Buffer.h"

#include <glad/glad.h>

std::string primitve_type_to_str(PrimitiveTypes pt)
{
	switch (pt)
	{
	case PrimitiveTypes::None:
		return "none";
		break;
	case PrimitiveTypes::Cube:
		return "cube";
		break;
	case PrimitiveTypes::Quad:
		return "quad";
		break;
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

Cube::Cube()
{
	std::vector<float> vertices =
	{
		-1.0f, -1.0f,  1.0f, // 0.f, 0.f, -1.f, 0.f, 1.f,		//        7--------6
		 1.0f, -1.0f,  1.0f, // 0.f, 0.f, -1.f, 0.f, 1.f,		//       /|       /|
		 1.0f, -1.0f, -1.0f, // 0.f, 0.f,  1.f, 0.f, 1.f,		//      4--------5 |
		-1.0f, -1.0f, -1.0f, // 0.f, 0.f,  1.f, 0.f, 1.f,		//      | |      | | 
		-1.0f,  1.0f,  1.0f, // 0.f, 0.f, -1.f, 0.f, 1.f,		//      | 3------|-2
		 1.0f,  1.0f,  1.0f, // 0.f, 0.f, -1.f, 0.f, 1.f,		//      |/       |/
		 1.0f,  1.0f, -1.0f, // 0.f, 0.f,  1.f, 0.f, 1.f,		//      0--------1
		-1.0f,  1.0f, -1.0f // 0.f, 0.f,  1.f, 0.f, 1.f
	};

	std::vector<unsigned int> indices =
	{
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

	m_cube_va.bind();

	VertexBuffer cube_vb(vertices);
	IndexBuffer cube_ib(indices);

	m_index_count = cube_ib.get_count();

	BufferLayout sb_layout = { 
		{0, 3, GL_FLOAT, false},
		//{1, 3, GL_FLOAT, false},
		//{2, 3, GL_FLOAT, false}
	};

	m_cube_va.set_layout(cube_vb, sb_layout);

	m_cube_va.unbind();
	cube_ib.unbind();
	cube_vb.unbind();
}

Quad::Quad()
{
	std::vector<float> vertices = {
		-0.5f, -0.5f, -0.5f, 0.f, 0.f, 1.f, 0.f, 0.f,
		-0.5f,	0.5f, -0.5f, 0.f, 0.f, 1.f, 0.f, 1.f,
		 0.5f,  0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 1.f,
		 0.5f, -0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f, 0.f
	};

	std::vector<unsigned int> indices = {
		0, 2, 1,
		0, 3, 2
	};

	m_quad_va.bind();

	VertexBuffer quad_vb(vertices);
	IndexBuffer quad_ib(indices);

	m_index_count = quad_ib.get_count();

	BufferLayout sb_layout = {
		{0, 3, GL_FLOAT, false},
		{1, 3, GL_FLOAT, false},
		{2, 2, GL_FLOAT, false}
	};

	m_quad_va.set_layout(quad_vb, sb_layout);

	m_quad_va.unbind();
	quad_ib.unbind();
	quad_vb.unbind();
}
