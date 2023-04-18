#pragma once

#include "renderer/Fwd.h"
#include "components/Component.h"
#include "Primitives.h"

#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class MeshView final : public Component
{
public:
    void set_mesh(const std::shared_ptr<Mesh>& mesh) { m_mesh = mesh; }
    [[nodiscard]] const std::shared_ptr<Mesh>& get_mesh() const { return m_mesh; }

    void set_mesh_info(const std::string& name, const std::string& type) { m_mesh_name = name; m_mesh_type = type; }
    void set_mesh_name(const std::string& name) { m_mesh_name = name; }

	void bind() const;
	void unbind() const;

    [[nodiscard]] const bool is_using_scale_outline() const { return m_use_scale_outline; }
    [[nodiscard]] const float get_scale_outline_factor() const { return m_outlining_factor; }

	[[nodiscard]] const char* get_name() const override { return "Mesh View"; }
	[[nodiscard]] size_t get_type() const override { return typeid(MeshView).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

    int m_instance_id = -1;

private:
    std::shared_ptr<Mesh> m_mesh;
    std::string m_mesh_name;
    std::string m_mesh_type; // TODO: think about using an enum

    // stenciling
    float m_outlining_factor = 0.02f;
    bool m_use_scale_outline = true;

	friend class SceneSerializer;
};

