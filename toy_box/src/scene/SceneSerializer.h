#pragma once

#include <memory>
#include <vector>

#include <json/json_fwd.hpp>

class Camera;
class Skybox;
class SceneNode;

class SceneSerializer
{
public:
	static void open(const char* scene, std::shared_ptr<Camera>& camera, std::unique_ptr<Skybox>& sky_box, SceneNode& entities);
	static void save(const char* scene, const std::shared_ptr<Camera>& camera, const std::unique_ptr<Skybox>& sky_box, const SceneNode& root);

private:
	static void load_skybox(nlohmann::json accessor, std::unique_ptr<Skybox>& sky_box);
	static void load_shaders(nlohmann::json accessor, unsigned int num_shaders);
	static void load_models(nlohmann::json accessor, unsigned int model_count, SceneNode& root);
	static SceneNode load_model(nlohmann::json accessor, int model_index, int& num_models_checked);
	static void serialize_node(nlohmann::json& accessor, int& node_index, const SceneNode& scene_node);
};

