#include "pch.h"
#include "SceneNode.hpp"
#include "Entity.hpp"
#include "components/Transform.h"

#include <stack>

SceneNode::~SceneNode()
{
	//m_entity.reset();
}

void SceneNode::set_entity(Entity &&e)
{
    // m_entity = std::make_shared<Entity>(e);
}

void SceneNode::add_child(SceneNode&& s)
{
//	m_children.emplace_back(std::make_shared<SceneNode>(std::move(s)));
//    m_children.back()->m_parent = this;
}

void SceneNode::add_child(SceneNode* s)
{
    children.emplace_back(s);
    children.back()->parent = this;
}

void SceneNode::move_child(SceneNode* s)
{
    if(this == s->parent) return;

    for (auto it = s->parent->children.begin(); it != s->parent->children.end(); ++it)
    {
        if (*it == s)
        {
            SceneNode* old_parent = s->parent;
            update_transform(s);
            add_child(s);
            old_parent->children.erase(it);
            break;
        }
    }
}

void SceneNode::update_transform(SceneNode* scene_node)
{
    auto& child_transform = scene_node->get_component<Transform>();
    Transform child_copy = child_transform; // needed to convert the nodes transform back to world space

    std::stack<Transform> transforms_to_apply;

    // goes up the parent chain to grab all the transforms needed to make the parent switch
    const auto& get_relative_transform = [&](SceneNode* current_parent)
    {
        Transform relative_transform{};

        while(current_parent)
        {
            const auto& transform_to_apply = current_parent->get_component<Transform>();
            transforms_to_apply.push(transform_to_apply);
            current_parent = current_parent->parent;
        }

        while(!transforms_to_apply.empty())
        {
            relative_transform = transforms_to_apply.top() * relative_transform;
            transforms_to_apply.pop();
        }

        return relative_transform;
    };

    SceneNode* parent1 = scene_node->parent; SceneNode* parent2 = this;
    Transform old_parent_transform = (scene_node->parent) ? get_relative_transform(parent1) : Transform{};
    Transform new_parent_transform = (scene_node->has_component<Transform>()) ? get_relative_transform(parent2) : Transform{};

    child_transform.resolve_parent_change(old_parent_transform, new_parent_transform);
    
    std::function<void (SceneNode*, Transform, Transform)> update_children = [&](SceneNode* child, const Transform& _old_parent_transform, const Transform& _new_parent_transform)
    {
        auto& _child_transform = child->get_component<Transform>();
        Transform _child_copy = _child_transform;

        _child_transform.resolve_parent_change(_old_parent_transform, _new_parent_transform);

        for(auto& c : child->children)
        {
            update_children(c, _old_parent_transform * _child_copy, _new_parent_transform * _child_transform);
        }
    };

    for(auto& child : scene_node->children)
    {
        update_children(child, old_parent_transform * child_copy, new_parent_transform * child_transform);
    }
}

bool SceneNode::exists(const std::string& name) const
{
	if (get_name() == name)
	{
		return true;
	}

	bool found = false;
	for (const auto& child : children)
	{
		found |= child->exists(name);
	}

	return found;
}

bool SceneNode::remove(SceneNode* node)
{
	auto it = children.begin();
	for (; it != children.end(); ++it)
	{
		if (*it == node)
		{
			children.erase(it);
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
	if (children.empty())
	{
		return 1;
	}

	size_t sz = 0;
	for (const auto& child : children)
	{
		sz += child->size();
	}

	return sz;
}
