#pragma once

class Entity;
class Transform;

class SceneNode
{
public:
	SceneNode() = default;
	SceneNode(std::unique_ptr<Entity>&& e);
	void add_child(SceneNode&& s);
	bool remove(SceneNode& node);
	SceneNode move(SceneNode& node);
	Transform update(const Transform& parent_transform);
	[[nodiscard]] bool exists(const std::string& name) const;
	[[nodiscard]] bool has_children() const { return (m_children.size() > 0); }
	[[nodiscard]] size_t size() const;
	
	inline std::vector<SceneNode>::iterator begin() { return m_children.begin(); }
	inline std::vector<SceneNode>::iterator end() { return m_children.end(); }
	inline std::vector<SceneNode>::const_iterator begin() const { return m_children.begin(); }
	inline std::vector<SceneNode>::const_iterator end() const { return m_children.end(); }

	bool operator== (const SceneNode& other)
	{
		return (entity == other.entity);
	}

	std::unique_ptr<Entity> entity;

private:
	std::vector<SceneNode> m_children;
	bool m_moved = false;
};

