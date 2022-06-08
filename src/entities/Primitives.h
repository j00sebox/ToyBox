#pragma once

#include "VertexArray.h"

// TODO: Add more
enum class PrimitiveTypes
{
	None = 0,
	Cube,
	Quad
};

std::string primitve_type_to_str(PrimitiveTypes pt);
PrimitiveTypes str_to_primitive_type(const char* name);

class Cube
{
public:
	Cube();

	[[nodiscard]] VertexArray& get_va() { return m_cube_va; }
	[[nodiscard]] unsigned int get_index_count() const { return m_index_count; }

private:
	VertexArray m_cube_va;
	unsigned int m_index_count;
};

class Quad
{
public:
	Quad();

	[[nodiscard]]VertexArray&& get_va() { return std::move(m_quad_va); }
	[[nodiscard]] unsigned int get_index_count() const { return m_index_count; }

private:
	VertexArray m_quad_va;
	unsigned int m_index_count;
};