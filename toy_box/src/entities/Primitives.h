#pragma once

#include "VertexArray.h"

// TODO: Add more
enum class PrimitiveTypes
{
	None = 0,
	Cube
};

class Cube
{
public:
	Cube();

	VertexArray&& get_va() { return std::move(m_cube_va); }
	unsigned int get_index_count() const { return m_index_count; }

private:
	VertexArray m_cube_va;
	unsigned int m_index_count;
};