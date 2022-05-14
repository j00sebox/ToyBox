#pragma once

#include "Primitives.h"
#include "renderer/Fwd.h"
#include "components/Component.h"

#include <mathz/Vector.h>
#include <string>

struct Vertex
{
	mathz::Vec3 positon;
	mathz::Vec3 normal;
	mathz::Vec2<float> st;
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

	[[nodiscard]] unsigned int get_index_count() const { return m_indices_count; }

	virtual void on_remove() {};
	[[nodiscard]] const char* get_name() const override { return "Mesh"; }
	[[nodiscard]] const char* get_type() const override { return typeid(Mesh).name(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

private:
	unsigned int m_indices_count = 0;
	VertexArray m_va;

	// TODO: Remove after a better way is found
	std::string m_gltf_path;
	PrimitiveTypes m_primitive = PrimitiveTypes::None;
	friend class SceneSerializer;
};

