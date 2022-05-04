#pragma once

#include <json/json.hpp>

using json = nlohmann::json;

class IComponent
{
public:
	virtual ~IComponent() = default;

	virtual void parse(json model) = 0;
	virtual void imgui_render() = 0;
};