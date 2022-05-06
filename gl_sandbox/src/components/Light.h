#pragma once

#include "IComponent.h"

#include "mathz/Vector.h"

class Light : public IComponent
{
public:
	Light();

	virtual void on_remove() override {};
	const char* get_type() const override { return typeid(Light).name(); }
	void parse(json info) override;
	void imgui_render() override;

	void set_colour(mathz::Vec4 col) { m_colour = col; }
	[[nodiscard]] const mathz::Vec4& get_colour() const { return m_colour; }

protected:
	mathz::Vec4 m_colour;
};

class PointLight : public Light
{
public:

	void on_remove() override;
	const char* get_type() const override { return typeid(PointLight).name(); }
	void parse(json info) override;
	void imgui_render() override;

private:
	float m_radius = 0.f;
};
