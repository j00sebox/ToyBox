#pragma once
#include "pch.h"

#include "Log.h"
#include "components/Component.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <mathz/Matrix.h>

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
	bool remove(const T& component)
	{
		if (m_type_map.find(component.get_type()) != m_type_map.end())
		{
			if (is_removeable(component.get_type()))
			{
				size_t index = m_type_map.at(component.get_type());

				m_components[m_type_map.at(component.get_type())]->on_remove();

				m_components.erase(m_components.begin() + index);

				m_type_map.erase(component.get_type());

				for (auto& [key, value] : m_type_map) {
					if (value > index)
					{
						--value;
					}
				}

				return true;
			}
		}

		return false;
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

	[[nodiscard]] std::vector<std::shared_ptr<Component>>& get_components() { return m_components; }

protected:
	bool is_removeable(const char* type)
	{
		return (!strcmp(type, "class Mesh") || !strcmp(type, "class Material") || !strcmp(type, "class PointLight") || !strcmp(type, "class DirectionalLight"));
	}

	std::string m_name;
	std::unordered_map<const char*, size_t> m_type_map;
	std::vector<std::shared_ptr<Component>> m_components;
};

