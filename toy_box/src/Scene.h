#pragma once

#include "Skybox.h"
#include "Camera.h"

class Entity;

class Scene
{
public:
	Scene();
	~Scene();

	void load(const char* scene);
	void save(const std::string& path);
	void init(int width, int height);
	void update(float elapsed_time);
	void render_components();
	void add_primitive(const char* name);
	void window_resize(int width, int height);
	void reset_view();
	Camera* get_camera() { return m_camera.get(); }

private:
	std::shared_ptr<Camera> m_camera;
	std::unique_ptr<Skybox> m_skybox;
	std::vector<std::unique_ptr<Entity>> m_entities;
	

	// imgui stuff
	Entity* m_selected_entity = nullptr;
};

