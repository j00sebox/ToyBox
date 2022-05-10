#pragma once

#include "Component.h"

#include "mathz/Vector.h"

class Light : public Component
{
public:
	Light();

	virtual void on_remove() override {};
	[[nodiscard]] const char* get_name() const override { return "Light"; }
	[[nodiscard]] const char* get_type() const override { return typeid(Light).name(); }
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
	void set_radius(float rad) { m_radius = rad; }
	void set_range(float range) { m_range = range; }
	[[nodiscard]] float get_radius() const { return m_radius; }
	[[nodiscard]] float get_range() const { return m_range; }

	void on_remove() override;
	[[nodiscard]] const char* get_name() const override { return "Point Light"; }
	[[nodiscard]] const char* get_type() const override { return typeid(PointLight).name(); }
	void parse(json info) override;
	void imgui_render() override;

private:
	float m_radius = 1.f;
	float m_range = 10.f;
};
