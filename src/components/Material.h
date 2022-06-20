#pragma once

#include "renderer/Fwd.h"
#include "components/Component.h"

#include "mathz/Vector.h"

#include <vector>
#include <memory>

class Material final : public Component
{
public:
	void set_shader(const std::shared_ptr<ShaderProgram>& shader) { m_shader = shader; }
	[[nodiscard]] const std::shared_ptr<ShaderProgram>& get_shader() const { return m_shader; }

	void load(const std::string* const textures);
	void bind() const;
	void unbind() const;

	void set_colour(const mathz::Vec4& colour) { m_colour = colour; }
	[[nodiscard]] const mathz::Vec4& get_colour() const { return m_colour; }
	[[nodiscard]] bool is_custom() const { return m_custom; }

	[[nodiscard]] const char* get_name() const override { return "Material"; }
	[[nodiscard]] size_t get_type() const override { return typeid(Material).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

private:
	std::shared_ptr<ShaderProgram> m_shader;
	bool m_custom = true;
	std::unique_ptr<Texture2D> m_textures[4];
	mathz::Vec4 m_colour;
	float m_metallic = 0.f;
	float m_roughness = 0.f;
};

