#pragma once

#include "Texture.h"
#include "Shader.h"
#include "GLTFLoader.h"

#include "components/IComponent.h"

#include <vector>
#include <memory>

class Material : public IComponent
{
public:
	void set_shader(const std::shared_ptr<ShaderProgram>& shader) { m_shader = shader; }
	[[nodiscard]] const std::shared_ptr<ShaderProgram>& get_shader() const { return m_shader; }

	void bind() const;
	void load(const GLTFLoader& loader);

	virtual void on_remove() override {};
	const char* get_type() const override { return typeid(Material).name(); }
	void parse(json info) override {};
	void imgui_render() override;

private:
	std::shared_ptr<ShaderProgram> m_shader;
	std::vector<Texture2D> m_textures;
};

