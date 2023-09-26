#pragma once
#include "CommonTypes.hpp"
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
    glm::mat4 transform;
    Mesh mesh;
    Material material;
    // MaterialComponent material;
    // unsigned instances = 1;
};

struct Skybox
{
    BufferHandle                    vertex_buffer;
    BufferHandle                    index_buffer;
    u32                             index_count;
    TextureHandle                   cubemap;
    DescriptorSetHandle             descriptor_set;
    DescriptorSetLayoutHandle       descriptor_set_layout;
    PipelineHandle                  pipeline;

    std::string                     resource_path;
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
    static void recompile_shaders() {}

	void set_background_colour(glm::vec4 colour);
	[[nodiscard]] const glm::vec4& get_background_colour() const { return m_clear_colour; }

    std::shared_ptr<Camera> camera;
    std::shared_ptr<Skybox> skybox;

    // LightManager m_light_manager;
    std::vector<RenderObject> m_render_list;

private:
    static void compile_shaders() {}

    // scene management
	void update_node(SceneNode* node, const glm::mat4& parent_transform);
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

