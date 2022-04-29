#pragma once

#include "Model.h"
#include "Skybox.h"
#include "Camera.h"

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
	std::shared_ptr<Camera> m_camera;
	std::unique_ptr<Skybox> m_skybox;
	std::vector<Model> m_models;
	mathz::Mat4 m_perspective;
};

