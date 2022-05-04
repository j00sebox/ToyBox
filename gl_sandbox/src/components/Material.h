#pragma once

#include "components/IComponent.h"

class Material : public IComponent
{
public:
	void set_shader(const std::string& shader) { m_shader_name = shader; }
	[[nodiscard]] const std::string& get_shader() const { return m_shader_name; }

	virtual void parse(json info) override;
	virtual void imgui_render() override;

private:
	std::string m_shader_name;
};

