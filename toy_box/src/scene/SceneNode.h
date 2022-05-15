#pragma once

class Entity;

class SceneNode
{
public:
	void add_child(std::unique_ptr<Entity>&& e);
	bool exists(const std::string& name) const;
	size_t size() const;
	
	[[nodiscard]] const std::string& get_name() const { return m_name; }
	
	inline std::vector<SceneNode>::iterator begin() { return m_children.begin(); }
	inline std::vector<SceneNode>::iterator end() { return m_children.end(); }
	inline std::vector<SceneNode>::const_iterator begin() const { return m_children.begin(); }
	inline std::vector<SceneNode>::const_iterator end() const { return m_children.end(); }

	std::unique_ptr<Entity> entity;

private:
	std::string m_name;
	std::vector<SceneNode> m_children;
};

