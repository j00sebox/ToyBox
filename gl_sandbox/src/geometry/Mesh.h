#pragma once

#include <string>
#include <memory>

#include "Texture.h"

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

	std::vector<float> get_vertices() const;
	std::vector<unsigned int> get_indices() const { return m_indices; }
	const std::shared_ptr<Texture2D> get_texture() const { return m_texture; }

private:
	std::vector<mathz::Vec3> floats_to_vec3(const std::vector<float>& flts) const;
	std::vector<mathz::Vec2<float>> floats_to_vec2(const std::vector<float>& flts) const;

	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
	std::shared_ptr<Texture2D> m_texture;
};

