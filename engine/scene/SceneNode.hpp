#pragma once
#include "Entity.hpp"

#include <list>
#include <vector>
#include <memory>

class Transform;
class SceneSerializer;

class SceneNode final : public Entity
{
public:
	SceneNode() = default;
	explicit SceneNode(std::shared_ptr<Entity>&& e);
    ~SceneNode();

    void set_entity(Entity&& e);
	void add_child(SceneNode&& s);
    void add_child(SceneNode* s);
    void move_child(SceneNode* s);
    void update_transform(SceneNode* scene_node);
	bool remove(SceneNode* node);

	[[nodiscard]] bool exists(const std::string& name) const;
	[[nodiscard]] bool has_children() const { return (!m_children.empty()); }
	[[nodiscard]] size_t size() const;

   // std::shared_ptr<Entity> entity() { return m_entity; }

	[[nodiscard]] inline std::vector<SceneNode*>::iterator begin() { return m_children.begin(); }
    [[nodiscard]] inline std::vector<SceneNode*>::iterator end() { return m_children.end(); }
    [[nodiscard]] inline std::vector<SceneNode*>::const_iterator begin() const { return m_children.begin(); }
    [[nodiscard]] inline std::vector<SceneNode*>::const_iterator end() const { return m_children.end(); }

//	bool operator== (const SceneNode& other) const
//	{
//		return (m_entity == other.m_entity);
//	}

    SceneNode* m_parent = nullptr;
    //   std::shared_ptr<Entity> m_entity;
    std::vector<SceneNode*> m_children;
};


