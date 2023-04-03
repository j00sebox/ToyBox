#pragma once

#include "renderer/Fwd.h"
#include "components/Component.h"
#include "Primitives.h"

#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class MeshObject final : public Component
{
public:
    void set_mesh(const std::shared_ptr<Mesh>& mesh) { m_mesh = mesh; }
    [[nodiscard]] const std::shared_ptr<Mesh>& get_mesh() const { return m_mesh; }

	void bind() const;
	void unbind() const;

	[[nodiscard]] const char* get_name() const override { return "MeshObject"; }
	[[nodiscard]] size_t get_type() const override { return typeid(MeshObject).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

    int m_instance_id = -1;

private:
    std::shared_ptr<Mesh> m_mesh;

	// TODO: Remove after a better way is found
	std::string m_gltf_path;
	PrimitiveTypes m_primitive = PrimitiveTypes::None;
	friend class SceneSerializer;
};

