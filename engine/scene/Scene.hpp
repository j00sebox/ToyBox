#pragma once
#include "Types.hpp"
// #include "Window.h"
#include "Camera.hpp"
// #include "Skybox.h"
#include "SceneNode.hpp"
//#include "LightManager.h"
#include "components/Fwd.h"
#include "components/Transform.h"

#include <map>
#include <queue>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class Entity;
class Buffer;
class Renderer;
struct RenderObject;


struct RenderObject
{
   // RenderCommand render_command;
    Transform transform;
    Mesh mesh;
    Material material;
    // MaterialComponent material;
    // unsigned instances = 1;
};

class Scene
{
public:
	explicit Scene();
	~Scene();

	void load(const char* scene);
    void close(Renderer* renderer);
	void save(const std::string& path);
	void init();
	void update(f32 elapsed_time);
	void add_primitive(const char* name) {}
    void add_model(const char* name) {}
	// void window_resize(int width, int height);
    static void recompile_shaders() {}

	void set_background_colour(glm::vec4 colour);
	[[nodiscard]] const glm::vec4& get_background_colour() const { return m_clear_colour; }

    // Window* m_window_handle;
    std::shared_ptr<Camera> camera;

    // LightManager m_light_manager;
    std::vector<RenderObject> m_render_list;

private:
    static void compile_shaders() {}

    // scene management
	void update_node(SceneNode* node, const Transform& parent_transform);
    void remove_node(SceneNode* node);
    void delete_node(Renderer* renderer, SceneNode* node);
   // std::unique_ptr<Skybox> m_skybox;
    // std::unique_ptr<Buffer> m_transforms_buffer;
    SceneNode root;
    std::queue<SceneNode*> m_nodes_to_remove;

    std::unordered_map<std::string, std::vector<glm::mat4>> instanced_meshes;
    // TODO: figure out better way
    std::unordered_map<std::string, bool> mesh_used;

    SceneNode* selectedNode = nullptr;
    glm::vec4 m_clear_colour = { 0.f, 0.f, 0.f, 1.f};

    friend class Inspector;
    friend class SceneSerializer;
};

