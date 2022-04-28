#pragma once

#include <string>

#include "Texture.h"
#include "VertexArray.h"
#include "Buffer.h"

#include "mathz/Vector.h"

struct Vertex
{
	mathz::Vec3 positon;
	mathz::Vec2<float> st;
};

class Mesh
{
public:
	Mesh(const std::string& fname);
	Mesh(Mesh&& mesh) noexcept;

	void draw() const;

private:
	std::vector<mathz::Vec3> floats_to_vec3(const std::vector<float>& flts) const;
	std::vector<mathz::Vec2<float>> floats_to_vec2(const std::vector<float>& flts) const;

	unsigned int m_indices_count;
	VertexArray m_va;
	Texture2D m_texture;
};

