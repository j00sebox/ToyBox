#pragma once

class IComponent
{
public:
	virtual ~IComponent() = default;

	void set_name(const char* name) { m_name = name; }
	[[nodiscard]] const char* get_name() const { m_name; }

private:
	const char* m_name;
};