#pragma once

#include "Skybox.h"
#include "Camera.h"
#include "SceneNode.h"

#include "components/Fwd.h"

#include <map>
#include <queue>

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
	void add_primitive(const char* name);
	void window_resize(int width, int height);
	void reset_view();
	Camera* get_camera() { return m_camera.get(); }

private:
	// scene management
	void create_light_map(const SceneNode& node);
	void remove_node(SceneNode& node);
	SceneNode move_node(SceneNode& node);

	// rendering helpers
	void update_node(SceneNode& node, const Transform& parent_transform);
	void update_lights();
	
	// imgui helpers
	void imgui_render(SceneNode& node);
	void display_components();

	std::shared_ptr<Camera> m_camera;
	std::unique_ptr<Skybox> m_skybox;
	SceneNode root;
	std::queue<SceneNode*> m_nodes_to_remove;
	std::vector<Entity*> m_point_lights;
	DirectionalLight* m_direct_light = nullptr;

	// imgui stuff
	SceneNode* m_selected_node = nullptr;
	SceneNode* m_drag_node = nullptr;
	SceneNode* m_drop_node = nullptr;
};

