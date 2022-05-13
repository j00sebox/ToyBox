#pragma once

#include "json/json_fwd.hpp"

class Component
{
public:
	virtual ~Component() = default;

	virtual void on_remove() = 0;
	virtual const char* get_name() const = 0;
	virtual const char* get_type() const = 0;
	virtual void imgui_render() = 0;
	virtual void serialize(nlohmann::json& accessor) const = 0;
};