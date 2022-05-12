#pragma once

class Component
{
public:
	virtual ~Component() = default;

	virtual void on_remove() = 0;
	virtual const char* get_name() const = 0;
	virtual const char* get_type() const = 0;
	virtual void imgui_render() = 0;
};