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

	virtual void on_remove() override {};
	[[nodiscard]] const char* get_name() const override { return "Material"; }
	[[nodiscard]] const char* get_type() const override { return typeid(Material).name(); }
	void parse(json info) override {};
	void imgui_render() override;

private:
	std::shared_ptr<ShaderProgram> m_shader;
	std::vector<Texture2D> m_textures;
};

