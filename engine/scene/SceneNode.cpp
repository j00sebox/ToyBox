#include "pch.h"
#include "SceneNode.hpp"
#include "Entity.hpp"
#include "components/Transform.h"

#include <stack>

SceneNode::SceneNode(std::shared_ptr<Entity>&& e)
{
    m_entity = std::move(e);
}

SceneNode::~SceneNode()
{
	m_entity.reset();
}

void SceneNode::set_entity(Entity &&e)
{
    m_entity = std::make_shared<Entity>(e);
}

void SceneNode::add_child(SceneNode&& s)
{
	m_children.emplace_back(std::make_shared<SceneNode>(std::move(s)));
    m_children.back()->m_parent = this;
}

void SceneNode::add_child(SceneNodePtr s)
{
    m_children.emplace_back(std::move(s));
    m_children.back()->m_parent = this;
}

void SceneNode::move_child(const SceneNodePtr& s)
{
    if(this == s->m_parent) return;

    for (auto it = s->m_parent->m_children.begin(); it != s->m_parent->m_children.end(); ++it)
    {
        if (*it == s)
        {
            SceneNode* old_parent = s->m_parent;
            update_transform(s);
            add_child(s);
            old_parent->m_children.erase(it);
            break;
        }
    }
}

void SceneNode::update_transform(const SceneNodePtr& s)
{
    auto& child_transform = s->m_entity->get_component<Transform>();
    Transform child_copy = child_transform; // needed to convert the nodes transform back to world space

    std::stack<Transform> transforms_to_apply;

    // goes up the parent chain to grab all the transforms needed to make the parent switch
    const auto& get_relative_transform = [&](SceneNode* current_parent)
    {
        Transform relative_transform{};

        while(current_parent && current_parent->m_entity)
        {
            const auto& transformToApply = current_parent->m_entity->get_component<Transform>();
            transforms_to_apply.push(transformToApply);
            current_parent = current_parent->m_parent;
        }

        while(!transforms_to_apply.empty())
        {
            relative_transform = transforms_to_apply.top() * relative_transform;
            transforms_to_apply.pop();
        }

        return relative_transform;
    };

    SceneNode* parent1 = s->m_parent; SceneNode* parent2 = this;
    Transform old_parent_transform = (s->m_parent && s->m_parent->m_entity) ? get_relative_transform(parent1) : Transform{};
    Transform new_parent_transform = (m_entity) ? get_relative_transform(parent2) : Transform{};

    child_transform.resolve_parent_change(old_parent_transform, new_parent_transform);
    
    std::function<void (SceneNodePtr&, Transform, Transform)> update_children = [&](SceneNodePtr& child, const Transform& _old_parent_transform, const Transform& _new_parent_transform)
    {
        auto& _child_transform = child->m_entity->get_component<Transform>();
        Transform _child_copy = _child_transform;

        _child_transform.resolve_parent_change(_old_parent_transform, _new_parent_transform);

        for(auto& c : child->m_children)
        {
            update_children(c, _old_parent_transform * _child_copy, _new_parent_transform * _child_transform);
        }
    };

    for(auto& child : s->m_children)
    {
        update_children(child, old_parent_transform * child_copy, new_parent_transform * child_transform);
    }
}

bool SceneNode::exists(const std::string& name) const
{
	if (m_entity && m_entity->get_name() == name)
	{
		return true;
	}

	bool found = false;
	for (const auto& child : m_children)
	{
		found |= child->exists(name);
	}

	return found;
}

bool SceneNode::remove(SceneNodePtr& node)
{
	auto it = m_children.begin();
	for (;it != m_children.end(); ++it)
	{
		if (*it == node)
		{
			m_children.erase(it);
			return true;
		}

		if ((*it)->has_children())
		{
			if ((*it)->remove(node))
				return true;
		}
	}

	return false;
}

size_t SceneNode::size() const
{
	if (m_children.empty())
	{
		return 1;
	}

	size_t sz = 0;
	for (const auto& child : m_children)
	{
		sz += child->size();
	}

	return sz;
}
