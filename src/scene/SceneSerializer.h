#pragma once

#include "components/Fwd.h"

#include <memory>
#include <vector>
#include <nlohmann/json_fwd.hpp>

class Camera;
class Skybox;
class Scene;
class SceneNode;

class SceneSerializer
{
public:
	static void open(const char* scene_name, Scene& scene, std::shared_ptr<Camera>& camera, std::unique_ptr<Skybox>& sky_box, SceneNode& entities);
	static void save(const char* scene_name, const Scene& scene, const std::shared_ptr<Camera>& camera, const std::unique_ptr<Skybox>& sky_box, const SceneNode& root);

private:
	static void load_skybox(const nlohmann::json& accessor, std::unique_ptr<Skybox>& sky_box);
	static void load_models(const nlohmann::json& accessor, unsigned int model_count, SceneNode& root, Scene& scene);
	static SceneNode load_model(const nlohmann::json& accessor, int model_index, int& num_models_checked, Scene& scene);
    static void load_material(nlohmann::json& accessor, Material& material);
	static void serialize_node(nlohmann::json& accessor, int& node_index, const SceneNode& scene_node);
};

