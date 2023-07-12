#include "pch.h"
#include "SceneNode.h"
#include "Entity.h"

#include "components/Transform.h"

SceneNode::SceneNode(SceneNode&& sn)
{
	entity = std::move(sn.entity);
    parent = sn.parent;
    sn.parent = nullptr;
	m_children = std::move(sn.m_children);
	sn.m_moved = true;
}

SceneNode::SceneNode(std::shared_ptr<Entity>&& e)
{
	entity = std::move(e);
}

SceneNode::~SceneNode()
{
	entity.reset();
}

void SceneNode::add_child(SceneNode&& s)
{
	s.m_moved = true;

    updateTransform(s);
    s.parent = this;
	m_children.emplace_back(std::move(s));
}

void SceneNode::addExistingChild(SceneNode& s)
{
    for (auto it = s.parent->m_children.begin(); it != s.parent->m_children.end(); ++it)
    {
        if (*it == s)
        {
            updateTransform(s);
            SceneNode* oldParent = s.parent;
            SceneNode sn{ std::move(*it) };
            sn.parent = this;
            m_children.push_back(std::move(sn));
            oldParent->m_children.erase(it);
            break;
        }
    }
}

void SceneNode::updateTransform(SceneNode& s)
{
    auto& childTransform = s.entity->get_component<Transform>();

    Transform oldParentTransform = (s.parent && s.parent->entity) ? s.parent->entity->get_component<Transform>() : Transform{};
    Transform newParentTransform = (entity) ? entity->get_component<Transform>() : Transform{};

    childTransform.resolveParentChange(oldParentTransform, newParentTransform);
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
	auto it = m_children.begin();
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
