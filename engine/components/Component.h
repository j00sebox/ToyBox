#pragma once

#include "nlohmann/json_fwd.hpp"

class Component
{
public:
	virtual ~Component() = default;

    [[nodiscard]] virtual const char* get_name() const = 0;
    [[nodiscard]] virtual size_t get_type() const = 0;
	virtual void imgui_render() = 0;
	virtual void serialize(nlohmann::json& accessor) const = 0;
};