#pragma once
#include "SceneNode.h"
#include "components/Fwd.h"

#include <array>
#include <list>
#include <memory>

class SceneNode;
class Entity;
class Camera;
class Buffer;
struct RenderObject;

class LightManager
{
public:
	LightManager();
	void get_lights(const SceneNodePtr& node);
    void init_lights();
	void update_lights(const std::vector<RenderObject>& render_list, const std::shared_ptr<Camera>& camera);

    // directional light
    void remove_directional_light();

    // point lights
    void add_point_light(const SceneNode& node);
	void remove_point_light(const SceneNodePtr& node);

private:
    void adjust_point_lights_buff();

	std::list<std::shared_ptr<Entity>> m_point_lights;
	std::shared_ptr<Entity> m_direct_light;
    std::unique_ptr<Buffer> m_point_light_buffer;
    std::unique_ptr<Buffer> m_direct_light_buffer;
    std::unique_ptr<Buffer> m_point_shadow_maps;
};