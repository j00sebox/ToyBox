#pragma once

#include <string>

#include "Texture.h"
#include "VertexArray.h"
#include "Primitives.h"

#include "components/Component.h"

#include <mathz/Vector.h>

struct Vertex
{
	mathz::Vec3 positon;
	mathz::Vec3 normal;
	mathz::Vec2<float> st;
};


class Mesh : public Component
{
public:
	Mesh() = default;
	Mesh(Mesh&& mesh) noexcept;

	void load(const std::vector<float>& verts, const std::vector<unsigned int>& indices);
	void load_primitive(PrimitiveTypes primitive);
	void bind() const;
	void unbind() const;

	[[nodiscard]] unsigned int get_index_count() const { return m_indices_count; }

	virtual void on_remove() {};
	[[nodiscard]] const char* get_name() const override { return "Mesh"; }
	[[nodiscard]] const char* get_type() const override { return typeid(Mesh).name(); }
	void imgui_render() override;

private:
	std::string m_path;
	unsigned int m_indices_count = 0;
	VertexArray m_va;
	std::vector<Texture2D> m_textures;
};

