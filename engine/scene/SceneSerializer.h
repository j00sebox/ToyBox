#pragma once

#include "components/Fwd.h"
#include "Material.h"
#include "SceneNode.h"

#include <memory>
#include <vector>
#include <nlohmann/json_fwd.hpp>

class Camera;
class Skybox;
class Scene;


class SceneSerializer
{
public:
	static void open(const char* scene_name, Scene& scene, std::shared_ptr<Camera>& camera, std::unique_ptr<Skybox>& sky_box, SceneNodePtr entities);
	static void save(const char* scene_name, const Scene& scene, const std::shared_ptr<Camera>& camera, const std::unique_ptr<Skybox>& sky_box, const SceneNodePtr& root);

private:
	static void load_skybox(const nlohmann::json& accessor, std::unique_ptr<Skybox>& sky_box);
	static void load_models(const nlohmann::json& accessor, unsigned int model_count, SceneNodePtr& root, Scene& scene);
	static SceneNodePtr load_model(const nlohmann::json& accessor, int model_index, int& num_models_checked, Scene& scene);
	static void serialize_node(nlohmann::json& accessor, int& node_index, const SceneNodePtr& scene_node);
};

