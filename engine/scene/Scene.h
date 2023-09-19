#pragma once

#include "Window.h"
#include "Camera.h"
#include "Skybox.h"
#include "SceneNode.h"
#include "LightManager.h"
#include "components/Fwd.h"

#include <map>
#include <queue>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Entity;
class Buffer;
struct RenderObject;

class Scene
{
public:
	explicit Scene(Window* window);
	~Scene();

	void load(const char* scene);
	void save(const std::string& path);
	void init();
	void update(float elapsed_time);
	void add_primitive(const char* name);
    void add_model(const char* name);
	void window_resize(int width, int height);
    static void recompile_shaders();

	void set_background_colour(glm::vec4 colour);
	[[nodiscard]] const glm::vec4& get_background_colour() const { return m_clear_colour; }

private:
    static void compile_shaders() ;

	// scene management
	void update_node(SceneNodePtr& node, const Transform& parent_transform);
	void remove_node(SceneNodePtr& node);

    Window* m_window_handle;
	std::shared_ptr<Camera> m_camera;
	std::unique_ptr<Skybox> m_skybox;
    std::unique_ptr<Buffer> m_transforms_buffer;
	SceneNodePtr root;
	std::queue<SceneNodePtr> m_nodes_to_remove;
	LightManager m_light_manager;
	std::vector<RenderObject> m_render_list;

    std::unordered_map<std::string, std::vector<glm::mat4>> instanced_meshes;
    // TODO: figure out better way
    std::unordered_map<std::string, bool> mesh_used;

    SceneNodePtr selectedNode = nullptr;
    glm::vec4 m_clear_colour = { 0.f, 0.f, 0.f, 1.f};

    friend class Inspector;
    friend class SceneSerializer;
};
