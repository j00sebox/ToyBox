#pragma once

class Entity;
class Transform;

#include <list>

class SceneNode
{
public:
	SceneNode() = default;
    SceneNode(SceneNode&& sn);
	SceneNode(std::shared_ptr<Entity>&& e);
    ~SceneNode();
	void add_child(SceneNode&& s);
    void addExistingChild(SceneNode& s);
    void updateTransform(SceneNode& s);
	bool remove(SceneNode& node);
	SceneNode move(SceneNode& node);
	Transform update(const Transform& parent_transform);
	[[nodiscard]] bool exists(const std::string& name) const;
	[[nodiscard]] bool has_children() const { return (!m_children.empty()); }
	[[nodiscard]] size_t size() const;
	
	[[nodiscard]] inline std::list<SceneNode>::iterator begin() { return m_children.begin(); }
    [[nodiscard]] inline std::list<SceneNode>::iterator end() { return m_children.end(); }
    [[nodiscard]] inline std::list<SceneNode>::const_iterator begin() const { return m_children.begin(); }
    [[nodiscard]] inline std::list<SceneNode>::const_iterator end() const { return m_children.end(); }

	bool operator== (const SceneNode& other) const
	{
		return (entity == other.entity);
	}

	std::shared_ptr<Entity> entity;

private:
	std::list<SceneNode> m_children;
    SceneNode* parent = nullptr;
	bool m_moved = false;
};

