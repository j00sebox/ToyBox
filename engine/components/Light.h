#pragma once

#include "Component.h"
#include "FrameBuffer.h"
#include "Texture.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

class Light : public Component
{
public:
    Light() :
    m_colour(1.f, 1.f, 1.f, 1.f),
    m_brightness(1.f),
    m_shadow_casting(false),
    m_shadow_width(2048),
    m_shadow_height(2048),
    m_shadow_bias(0.05f)
    {}
	void set_colour(glm::vec4 col) { m_colour = col; }
	void set_brightness(float b) { m_brightness = b; }
    void cast_shadow() { m_shadow_casting = true; }
	[[nodiscard]] const glm::vec4& get_colour() const { return m_colour; }
	[[nodiscard]] float get_brightness() const { return m_brightness; }
    [[nodiscard]] bool is_casting_shadow() const { return m_shadow_casting; }
    [[nodiscard]] std::pair<unsigned int, unsigned int> get_shadow_dimensions() const { return { m_shadow_width, m_shadow_height }; }
    [[nodiscard]] float get_shadow_bias() const { return m_shadow_bias; }

	[[nodiscard]] const char* get_name() const override { return "Light"; }
	[[nodiscard]] size_t get_type() const override { return typeid(Light).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override = 0;

    virtual void shadow_init(const glm::vec3& light_pos) = 0;

protected:
    glm::vec4 m_colour;
	float m_brightness;
    bool m_shadow_casting;
    unsigned m_shadow_width, m_shadow_height;
    float m_shadow_bias;
};

class DirectionalLight final : public Light
{
public:
	void set_direction(const glm::vec3& dir) { m_direction = dir; glm::normalize(m_direction); }
	[[nodiscard]] const glm::vec3& get_direction() const { return m_direction; }

    [[nodiscard]] const glm::mat4& get_light_view() const { return m_light_view; }
    [[nodiscard]] const glm::mat4& get_light_projection() const { return m_light_projection; }

    void bind_shadow_map() const { m_shadow_map->bind(); };
    [[nodiscard]] const std::shared_ptr<FrameBuffer>& get_shadow_buffer() const { return m_shadow_map; };
    [[nodiscard]] unsigned int get_shadow_map() const { return m_shadow_map->get_depth_attachment(); }

	[[nodiscard]] const char* get_name() const override { return "Directional Light"; }
	[[nodiscard]] size_t get_type() const override { return typeid(DirectionalLight).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

    void shadow_init(const glm::vec3& light_pos) override;

private:
    glm::vec3 m_direction;

    // shadow related stuff
    std::shared_ptr<FrameBuffer> m_shadow_map;
    glm::mat4 m_light_projection;
    glm::mat4 m_light_view;
};

class PointLight final : public Light
{
public:
    PointLight() :
    m_range(10.f),
    m_shadow_near(1.f),
    m_shadow_far(100.f),
    m_shadow_proj(glm::mat4(1.f)),
    m_shadow_info_change(false)
    {}
	void set_range(float range) { m_range = range; }
	[[nodiscard]] float get_range() const { return m_range; }

    void bind_shadow_map() const { m_shadow_map->bind(); };
    [[nodiscard]] unsigned get_shadow_cubemap() const { return m_shadow_map->get_depth_attachment(); };
    [[nodiscard]] bool has_shadow_info_changed() const { return m_shadow_info_change; }
    [[nodiscard]] float get_far_plane() const { return m_shadow_far; }
    [[nodiscard]] std::vector<glm::mat4>& get_shadow_transforms() { return m_shadow_transforms; }
    [[nodiscard]] const std::shared_ptr<FrameBuffer>& get_shadow_buffer() const { return m_shadow_map; };

    void shadow_init(const glm::vec3& light_pos) override;
    void shadow_resize(const glm::vec3& light_pos);
    void shadow_update_transforms(const glm::vec3& light_pos);

	[[nodiscard]] const char* get_name() const override { return "Point Light"; }
	[[nodiscard]] size_t get_type() const override { return typeid(PointLight).hash_code(); }
	void imgui_render() override;
	void serialize(nlohmann::json& accessor) const override;

private:
	float m_range;

    // shadow info
    std::shared_ptr<FrameBuffer> m_shadow_map;
    float m_shadow_near, m_shadow_far;
    glm::mat4 m_shadow_proj;
    std::vector<glm::mat4> m_shadow_transforms;
    bool m_shadow_info_change;

};
