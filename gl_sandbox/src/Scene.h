#pragma once

#include "Model.h"
#include "Skybox.h"
#include "Camera.h"

#include <json/json.hpp>

using json = nlohmann::json;

class Scene
{
public:
	void set_perspective(const mathz::Mat4& perspective);
	void load(const char* scene);
	void draw(const mathz::Mat4& look_at, const mathz::Mat4& look_at_no_t);

private:
	json m_json;
	std::unique_ptr<Skybox> m_skybox;
	std::vector<Model> m_models;
	mathz::Mat4 m_perspective;
};

