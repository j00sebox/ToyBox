#pragma once

#include <memory>
#include <vector>

#include <json/json_fwd.hpp>

class Entity;
class Camera;
class Skybox;
class SceneNode;

class SceneSerializer
{
public:
	static void open(const char* scene, std::shared_ptr<Camera>& camera, std::unique_ptr<Skybox>& sky_box, std::vector<std::unique_ptr<Entity>>& entities);
	static void save(const char* scene, const std::shared_ptr<Camera>& camera, const std::unique_ptr<Skybox>& sky_box, const SceneNode& root);

private:
	static void load_skybox(nlohmann::json accessor, std::unique_ptr<Skybox>& sky_box);
	static void load_shaders(nlohmann::json accessor, unsigned int num_shaders);
	static void load_models(nlohmann::json accessor, unsigned int num_models, std::vector<std::unique_ptr<Entity>>& entities);
};

