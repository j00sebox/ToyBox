#pragma once
#include "Types.hpp"
// #include "components/Fwd.h"
// #include "Material.h"
#include "SceneNode.hpp"

#include <memory>
#include <vector>
#include <nlohmann/json_fwd.hpp>

class Camera;
class Skybox;
class Scene;
class Renderer;

class SceneSerializer
{
public:
	static void open(const char* scene_name, Scene* scene, Renderer* renderer);
	static void save(const char* scene_name, const Scene& scene, const std::shared_ptr<Camera>& camera, const std::unique_ptr<Skybox>& sky_box, const SceneNodePtr& root);

private:
	static void load_skybox(const nlohmann::json& accessor, std::unique_ptr<Skybox>& sky_box);
	static void load_models(const nlohmann::json& accessor, u32 model_count, SceneNodePtr& root, Scene* scene, Renderer* renderer);
	static SceneNodePtr load_model(const nlohmann::json& accessor, u32 model_index, u32& num_models_checked, Scene* scene, Renderer* renderer);
	static void serialize_node(nlohmann::json& accessor, int& node_index, const SceneNodePtr& scene_node);
};

