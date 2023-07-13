#pragma once

class Entity;
class Transform;
class SceneSerializer;

#include <list>

class SceneNode;
typedef std::shared_ptr<SceneNode> SceneNodePtr;

class SceneNode
{
public:
	SceneNode() = default;
	SceneNode(std::shared_ptr<Entity>&& e);
    ~SceneNode();

	void add_child(SceneNode&& s);
    void addChild(SceneNodePtr s);
    void addExistingChild(SceneNodePtr s);
    void updateTransform(SceneNodePtr s);
	bool remove(SceneNodePtr& node);
	SceneNode move(SceneNodePtr node);
	Transform update(const Transform& parent_transform);
	[[nodiscard]] bool exists(const std::string& name) const;
	[[nodiscard]] bool has_children() const { return (!m_children.empty()); }
	[[nodiscard]] size_t size() const;

	[[nodiscard]] inline std::vector<SceneNodePtr>::iterator begin() { return m_children.begin(); }
    [[nodiscard]] inline std::vector<SceneNodePtr>::iterator end() { return m_children.end(); }
    [[nodiscard]] inline std::vector<SceneNodePtr>::const_iterator begin() const { return m_children.begin(); }
    [[nodiscard]] inline std::vector<SceneNodePtr>::const_iterator end() const { return m_children.end(); }

	bool operator== (const SceneNode& other) const
	{
		return (entity == other.entity);
	}

	std::shared_ptr<Entity> entity;

private:
	std::vector<SceneNodePtr> m_children;
    SceneNode* parent = nullptr;

    friend class SceneSerializer;
};


