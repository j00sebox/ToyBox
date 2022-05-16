#include "pch.h"
#include "SceneNode.h"

#include "Entity.h"

SceneNode::SceneNode(std::unique_ptr<Entity>&& e)
{
	m_name = e->get_name();
	entity = std::move(e);
}

void SceneNode::add_child(SceneNode&& s)
{
	m_children.emplace_back(std::move(s));
}

bool SceneNode::exists(const std::string& name) const
{
	if (m_name == name)
	{
		return true;
	}

	bool found = false;
	for (const SceneNode& sn : m_children)
	{
		found |= sn.exists(name);
	}

	return found;
}

size_t SceneNode::size() const
{
	if (m_children.size() == 0)
	{
		return 1;
	}

	size_t sz = 0;
	for (const SceneNode& sn : m_children)
	{
		sz += sn.size();
	}

	return sz;
}
