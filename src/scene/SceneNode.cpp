#include "pch.h"
#include "SceneNode.h"

#include "Entity.h"

#include "components/Transform.h"

SceneNode::SceneNode(std::unique_ptr<Entity>&& e)
{
	entity = std::move(e);
}

void SceneNode::add_child(SceneNode&& s)
{
	s.m_moved = true;
	m_children.emplace_back(std::move(s));
}

bool SceneNode::exists(const std::string& name) const
{
	if (entity && entity->get_name() == name)
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

bool SceneNode::remove(SceneNode& node)
{
	std::list<SceneNode>::iterator it = m_children.begin();
	for (;it != m_children.end(); ++it)
	{
		if (*it == node)
		{
			m_children.erase(it);
			return true;
		}

		if (it->has_children())
		{
			if (it->remove(node))
				return true;
		}
	}

	return false;
}

// node must exist for this to work
SceneNode SceneNode::move(SceneNode& node)
{
	std::list<SceneNode>::iterator it = m_children.begin();
	for (; it != m_children.end(); ++it)
	{
		if (*it == node)
		{
			SceneNode sn{ std::move(*it) };
			m_children.erase(it);
			return sn;
		}

		if (it->has_children())
		{
			SceneNode sn = it->move(node);
			if (sn.entity)
				return sn;
		}
	}

	return SceneNode{};
}

Transform SceneNode::update(const Transform& parent_transform)
{
	auto& transform = entity->get_component<Transform>();
	// this is only true if the node was recently given a new parent
	if (m_moved)
	{
		// need to adjust this if the parent is new so it doesn't modify the transform
		transform.translate((parent_transform.get_position() * -1.f) + transform.get_position() + transform.get_parent_pos());
		transform.scale((1 / parent_transform.get_uniform_scale()) * transform.get_uniform_scale() * transform.get_parent_scale());
		m_moved = false;
	}
	transform.set_parent_offsets(parent_transform.get_position(), parent_transform.get_uniform_scale()); // update so that it will be correct size after switching parents
	return  parent_transform * transform;
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
