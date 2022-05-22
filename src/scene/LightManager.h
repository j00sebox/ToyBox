#pragma once

#include "components/Fwd.h"

#include <array>
#include <queue>

class SceneNode;
class Entity;
class Camera;

#define MAX_POINT_LIGHTS 2

class LightManager
{
public:
	LightManager();
	void set_lights(const SceneNode& node);
	void update_lights(const std::shared_ptr<Camera>& camera);
	void add_point_light(const SceneNode& node);
	void remove_point_light(const SceneNode& node);

private:
	std::array<Entity*, MAX_POINT_LIGHTS> m_point_lights;
	DirectionalLight* m_direct_light = nullptr;
	std::queue<int> m_available_point_lights;
};