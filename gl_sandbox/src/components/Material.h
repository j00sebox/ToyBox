#pragma once

#include "Texture.h"
#include "Shader.h"
#include "GLTFLoader.h"

#include "components/Component.h"

#include <vector>
#include <memory>

class Material : public Component
{
public:
	void set_shader(const std::shared_ptr<ShaderProgram>& shader) { m_shader = shader; }
	[[nodiscard]] const std::shared_ptr<ShaderProgram>& get_shader() const { return m_shader; }

	void load(const GLTFLoader& loader);
	void bind() const;

	void set_colour(const mathz::Vec4& colour) { m_colour = colour; }
	[[nodiscard]] const mathz::Vec4& get_colour() const { return m_colour; }
	[[nodiscard]] bool is_using_colour() const { return m_use_colour; }

	virtual void on_remove() override {};
	[[nodiscard]] const char* get_name() const override { return "Material"; }
	[[nodiscard]] const char* get_type() const override { return typeid(Material).name(); }
	void parse(json info) override {};
	void imgui_render() override;

private:
	std::shared_ptr<ShaderProgram> m_shader;
	bool m_use_colour = true;
	std::unique_ptr<Texture2D> m_textures[4];
	mathz::Vec4 m_colour;
};

