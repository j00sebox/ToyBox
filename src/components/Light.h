#pragma once

#include "Component.h"

#include "FrameBuffer.h"

#include <mathz/Vector.h>

class Light : public Component
{
public:
    Light();
	void set_colour(mathz::Vec4 col) { m_colour = col; }
	void set_brightness(float b) { m_brightness = b; }
	[[nodiscard]] const mathz::Vec4& get_colour() const { return m_colour; }
	[[nodiscard]] float get_brightness() const { return m_brightness; }

	[[nodiscard]] const char* get_name() const override { return "Light"; }
	[[nodiscard]] size_t get_type() const override { return typeid(Light).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override = 0;

protected:
    virtual void shadow_init() = 0;

    mathz::Vec4 m_colour;
	float m_brightness = 1.f;

    // shadow related stuff
    bool m_shadow_casting;
    std::shared_ptr<FrameBuffer> m_shadow_map;
    static const unsigned int m_shadow_width = 1024, m_shadow_height = 1024;
};

class DirectionalLight final : public Light
{
public:
	void set_direction(const mathz::Vec3& dir) { m_direction = dir; m_direction.normalize(); }
	[[nodiscard]] const mathz::Vec3& get_direction() const { return m_direction; }

	[[nodiscard]] const char* get_name() const override { return "Directional Light"; }
	[[nodiscard]] size_t get_type() const override { return typeid(DirectionalLight).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

protected:
    void shadow_init() override;

private:
	mathz::Vec3 m_direction;
};

class PointLight final : public Light
{
public:
	void set_radius(float rad) { m_radius = rad; }
	void set_range(float range) { m_range = range; }
	[[nodiscard]] float get_radius() const { return m_radius; }
	[[nodiscard]] float get_range() const { return m_range; }

	[[nodiscard]] const char* get_name() const override { return "Point Light"; }
	[[nodiscard]] size_t get_type() const override { return typeid(PointLight).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

protected:
    void shadow_init() override {};

private:
	float m_radius = 1.f;
	float m_range = 10.f;

	friend class SceneSerializer;
};
