#include "pch.h"
#include "SceneNode.h"
#include "Entity.h"
#include "components/Transform.h"

#include <stack>

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
	m_children.emplace_back(std::make_shared<SceneNode>(std::move(s)));
    m_children.back()->parent = this;
}

void SceneNode::addChild(SceneNodePtr s)
{
    m_children.emplace_back(std::move(s));
    m_children.back()->parent = this;
}

void SceneNode::addExistingChild(SceneNodePtr s)
{
    if(this == s->parent) return;

    for (auto it = s->parent->m_children.begin(); it != s->parent->m_children.end(); ++it)
    {
        if (*it == s)
        {
            SceneNode* oldParent = s->parent;
            updateTransform(s);
            addChild(s);
            oldParent->m_children.erase(it);
            break;
        }
    }
}

// TODO: add comments
void SceneNode::updateTransform(SceneNodePtr s)
{
    auto& childTransform = s->entity->get_component<Transform>();
    Transform childCopy = childTransform;

    std::stack<Transform> transformsToApply;

    const auto& getRelativeTransform = [&](SceneNode* currentParent)
    {
        Transform relativeTransform{};

        while(currentParent && currentParent->entity)
        {
            const auto& transformToApply = currentParent->entity->get_component<Transform>();
            transformsToApply.push(transformToApply);
            currentParent = currentParent->parent;
        }

        while(!transformsToApply.empty())
        {
            relativeTransform = transformsToApply.top() * relativeTransform;
            transformsToApply.pop();
        }

        return relativeTransform;
    };

    SceneNode* parent1 = s->parent; SceneNode* parent2 = this;
    Transform oldParentTransform = (s->parent && s->parent->entity) ? getRelativeTransform(parent1) : Transform{};
    Transform newParentTransform = (entity) ? getRelativeTransform(parent2) : Transform{};

    childTransform.resolveParentChange(oldParentTransform, newParentTransform);
    
    std::function<void (SceneNodePtr&, Transform, Transform)> updateChildren = [&](SceneNodePtr& child, Transform oldPT, Transform newPT)
    {
        auto& cTransform = child->entity->get_component<Transform>();
        Transform cCopy = cTransform;

        cTransform.resolveParentChange(oldPT, newPT);

        for(auto& c : child->m_children)
        {
            updateChildren(c, oldPT * cCopy, newPT * cTransform);
        }
    };

    for(auto& child : s->m_children)
    {
        updateChildren(child, oldParentTransform * childCopy, newParentTransform * childTransform);
    }
}

bool SceneNode::exists(const std::string& name) const
{
	if (entity && entity->get_name() == name)
	{
		return true;
	}

	bool found = false;
	for (auto child : m_children)
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

// node must exist for this to work
SceneNode SceneNode::move(SceneNodePtr node)
{
//	auto it = m_children.begin();
//	for (; it != m_children.end(); ++it)
//	{
//		if (*it == node)
//		{
//			SceneNode sn{ std::move(*it) };
//			m_children.erase(it);
//			return sn;
//		}
//
//		if (it->has_children())
//		{
//			SceneNode sn = it->move(node);
//			if (sn.entity)
//				return sn;
//		}
//	}
//
//	return SceneNode{};
}

size_t SceneNode::size() const
{
	if (m_children.size() == 0)
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
