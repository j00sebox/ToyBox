#pragma once

#include "Skybox.h"
#include "Camera.h"
#include "SceneNode.h"

#include <map>

class Entity;
class Transform;

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
	void update_node(SceneNode& node, const Transform& parent_transform);

	std::shared_ptr<Camera> m_camera;
	std::unique_ptr<Skybox> m_skybox;
	std::vector<std::unique_ptr<Entity>> m_entities;
	std::map<std::string, std::unique_ptr<Entity>> m_es;
	SceneNode root;

	// imgui stuff
	SceneNode* m_selected_node = nullptr;
};

