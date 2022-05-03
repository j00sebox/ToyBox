#pragma once

class IComponent
{
public:
	virtual ~IComponent() = default;

	void set_name(const char* name) { m_name = name; }
	[[nodiscard]] const char* get_name() const { m_name; }

	virtual void imgui_render() = 0;

protected:
	const char* m_name;
};