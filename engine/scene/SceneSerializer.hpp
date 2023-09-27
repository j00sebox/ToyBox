#pragma once
#include "CommonTypes.hpp"
// #include "components/Fwd.h"
// #include "Material.h"
#include "SceneNode.hpp"

#include <memory>
#include <vector>
#include <nlohmann/json_fwd.hpp>

struct Skybox;
class Camera;
class Scene;
class Renderer;

class SceneSerializer
{
public:
	static void open(const char* scene_name, Scene* scene, Renderer* renderer);
	static void save(const char* scene_name,  const Scene* scene);

    static void load_model(SceneNode* scene_node, const char* model_path);
    static void load_primitive(SceneNode* scene_node, const char* primitive_name, Renderer* renderer);

private:
    static Skybox load_skybox(const nlohmann::json& accessor, Renderer* renderer);
    static void load_nodes(const nlohmann::json& accessor, u32 model_count, Scene* scene, Renderer* renderer);
    static SceneNode* load_node(const nlohmann::json& accessor, u32 model_index, u32& num_models_checked, Scene* scene, Renderer* renderer);

	static void serialize_node(nlohmann::json& accessor, int& node_index, const SceneNode* scene_node);
};

