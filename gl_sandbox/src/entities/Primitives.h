#pragma once

#include "VertexArray.h"

// TODO: Add more
enum class PrimitiveTypes
{
	Cube = 0
};

class Cube
{
public:
	Cube();

private:
	VertexArray m_cube_va;
	unsigned int m_index_count;
};