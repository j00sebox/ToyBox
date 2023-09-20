#pragma once

#include <list>
#include <vector>
#include <memory>

class Entity;
class Transform;
class SceneSerializer;
class SceneNode;
typedef std::shared_ptr<SceneNode> SceneNodePtr;

class SceneNode
{
public:
	SceneNode() = default;
	explicit SceneNode(std::shared_ptr<Entity>&& e);
    ~SceneNode();

    void set_entity(Entity&& e);
	void add_child(SceneNode&& s);
    void add_child(SceneNodePtr s);
    void move_child(const SceneNodePtr& s);
    void update_transform(const SceneNodePtr& current_parent);
	bool remove(SceneNodePtr& node);

	[[nodiscard]] bool exists(const std::string& name) const;
	[[nodiscard]] bool has_children() const { return (!m_children.empty()); }
	[[nodiscard]] size_t size() const;

    std::shared_ptr<Entity> entity() { return m_entity; }

	[[nodiscard]] inline std::vector<SceneNodePtr>::iterator begin() { return m_children.begin(); }
    [[nodiscard]] inline std::vector<SceneNodePtr>::iterator end() { return m_children.end(); }
    [[nodiscard]] inline std::vector<SceneNodePtr>::const_iterator begin() const { return m_children.begin(); }
    [[nodiscard]] inline std::vector<SceneNodePtr>::const_iterator end() const { return m_children.end(); }

	bool operator== (const SceneNode& other) const
	{
		return (m_entity == other.m_entity);
	}

private:
    std::shared_ptr<Entity> m_entity;
	std::vector<SceneNodePtr> m_children;
    SceneNode* m_parent = nullptr;
};


