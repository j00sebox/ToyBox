#pragma once

#include "Model.h"
#include "Skybox.h"
#include "Camera.h"

#include <json/json.hpp>

using namespace nlohmann;

class Scene
{
public:
	Scene();
	~Scene();

	void load(const char* scene);
	void init(int width, int height);
	void update(float elapsed_time);
	void render_components();
	void reset_view();
	Camera* get_camera() { return m_camera.get(); }

private:
	json m_json;
	std::shared_ptr<Camera> m_camera;
	std::unique_ptr<Skybox> m_skybox;
	std::vector<std::unique_ptr<Entity>> m_entities;
	mathz::Mat4 m_perspective;

	// imgui stuff
	Entity* m_selected_entity = nullptr;
};

