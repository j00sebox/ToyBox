#pragma once

#include "components/Fwd.h"

#include <array>
#include <queue>

class SceneNode;
class Entity;
class Camera;
class UniformBuffer;
struct RenderObject;

#define MAX_POINT_LIGHTS 2

class LightManager
{
public:
	LightManager();
	void set_lights(const SceneNode& node);
	void update_lights(const std::vector<RenderObject>& render_list, const std::shared_ptr<Camera>& camera);

    // directional light
    void set_directional_light(const DirectionalLight& dl);
    void remove_directional_light();

    // point lights
    void add_point_light(const SceneNode& node);
	void remove_point_light(const SceneNode& node);

private:
	std::array<Entity*, MAX_POINT_LIGHTS> m_point_lights;
	std::shared_ptr<DirectionalLight> m_direct_light;
    std::queue<int> m_available_point_lights;
    std::unique_ptr<UniformBuffer> m_light_uniform_buffer;
};