#pragma once

#include "Model.h"
#include "Skybox.h"
#include "Camera.h"
#include "lights/PointLight.h"

#include <json/json.hpp>

using json = nlohmann::json;

class Scene
{
public:
	Scene();

	void load(const char* scene);
	void init();
	void update(float elapsed_time);
	void draw();
	Camera* get_camera() { return m_camera.get(); }

private:
	json m_json;
	ShaderLibrary m_shader_lib;
	std::shared_ptr<Camera> m_camera;
	std::unique_ptr<Skybox> m_skybox;
	std::vector<std::unique_ptr<Entity>> m_objects;
	PointLight m_point_light;
	mathz::Vec3 m_directional_light;
	mathz::Mat4 m_perspective;

	// imgui stuff
	Entity* m_selected_object = nullptr;
};

