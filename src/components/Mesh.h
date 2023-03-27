#pragma once

#include "Primitives.h"
#include "renderer/Fwd.h"
#include "components/Component.h"

#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Vertex
{
	glm::vec3 positon;
	glm::vec3 normal;
	glm::vec2 st;
};

class Mesh final : public Component
{
public:
	Mesh() = default;
	Mesh(Mesh&& mesh) noexcept;

	void load(const std::vector<float>& verts, const std::vector<unsigned int>& indices);
	void load_primitive(PrimitiveTypes primitive);
	void bind() const;
	void unbind() const;

    [[nodiscard]] unsigned int get_vertex_count() const { return m_vertex_count; }
	[[nodiscard]] unsigned int get_index_count() const { return m_indices_count; }

	[[nodiscard]] const char* get_name() const override { return "Mesh"; }
	[[nodiscard]] size_t get_type() const override { return typeid(Mesh).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

private:
    unsigned int m_vertex_count = 0;
	unsigned int m_indices_count = 0;
	VertexArray m_va;
    VertexBuffer m_vb;
    IndexBuffer m_ib;

	// TODO: Remove after a better way is found
	std::string m_gltf_path;
	PrimitiveTypes m_primitive = PrimitiveTypes::None;
	friend class SceneSerializer;
};

