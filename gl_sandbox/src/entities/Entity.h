#pragma once
#include "pch.h"

#include "Log.h"
#include "components/IComponent.h"

#include "mathz/Matrix.h"

class Entity
{
public:
	virtual void set_name(const std::string& name) { m_name = name; }
	[[nodiscard]] const std::string& get_name() const { return m_name; }

	template<class T>
	void attach(T&& component)
	{
		const char* type_name = typeid(T).name();
		if (m_type_map.find(type_name) == m_type_map.end())
		{
			m_components.emplace_back(std::make_shared<T>(std::forward<T>(component)));
			m_type_map[type_name] = m_components.size() - 1;
		}
	}

	template<class T>
	T& get() const
	{
		const char* type_name = typeid(T).name();
		if (m_type_map.find(type_name) == m_type_map.end())
		{
			fprintf(stderr, "Component does not exist!\n");
			ASSERT(false);
		}

		return *std::static_pointer_cast<T>(m_components[m_type_map.at(type_name)]);
	}
	
	template<class T>
	bool has() const
	{
		return (m_type_map.find(typeid(T).name()) != m_type_map.end());
	}

	void render_components()
	{
		for (const std::shared_ptr<IComponent>& component : m_components)
		{
			component->imgui_render();
		}
	}

protected:
	std::string m_name;
	std::unordered_map<const char*, int> m_type_map;
	std::vector<std::shared_ptr<IComponent>> m_components;
};

