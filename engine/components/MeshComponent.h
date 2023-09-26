#pragma once
#include "CommonTypes.hpp"
// #include "renderer/Fwd.h"
#include "components/Component.h"
#include "Primitives.hpp"

#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

class MeshComponent final : public Component
{
public:
    void set_mesh(const std::shared_ptr<Mesh>& _mesh) { m_mesh = _mesh; }
    [[nodiscard]] const std::shared_ptr<Mesh>& get_mesh() const { return m_mesh; }
    [[nodiscard]] const Mesh& get() { return m_mesh.operator*(); }

    void set_mesh_info(const std::string& name, const std::string& type) { m_mesh_name = name; m_mesh_type = type; }
    void set_mesh_name(const std::string& name) { m_mesh_name = name; }

	void bind() const;
	void unbind() const;

    [[nodiscard]] bool is_using_scale_outline() const { return m_use_scale_outline; }
    [[nodiscard]] float get_scale_outline_factor() const { return m_outlining_factor; }

	[[nodiscard]] const char* get_name() const override { return "Mesh"; }
	[[nodiscard]] size_t get_type() const override { return typeid(MeshComponent).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

    int m_instance_id = -1;
    Mesh mesh;
    Material material;

private:
    std::shared_ptr<Mesh> m_mesh;
    std::string m_mesh_name;
    std::string m_mesh_type; // TODO: think about using an enum

    // stenciling
    float m_outlining_factor = 0.02f;
    bool m_use_scale_outline = true;

	friend class SceneSerializer;
};

