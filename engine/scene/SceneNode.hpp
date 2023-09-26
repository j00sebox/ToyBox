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
    ~SceneNode();

    void set_entity(Entity&& e);
	void add_child(SceneNode&& s);
    void add_child(SceneNode* s);
    void move_child(SceneNode* s);
    void update_transform(SceneNode* scene_node);
	bool remove(SceneNode* node);

	[[nodiscard]] bool exists(const std::string& name) const;
	[[nodiscard]] bool has_children() const { return (!children.empty()); }
	[[nodiscard]] size_t size() const;

	[[nodiscard]] inline std::vector<SceneNode*>::iterator begin() { return children.begin(); }
    [[nodiscard]] inline std::vector<SceneNode*>::iterator end() { return children.end(); }
    [[nodiscard]] inline std::vector<SceneNode*>::const_iterator begin() const { return children.begin(); }
    [[nodiscard]] inline std::vector<SceneNode*>::const_iterator end() const { return children.end(); }

    SceneNode* parent = nullptr;
    std::vector<SceneNode*> children;
    bool is_locked = false;
    bool is_model_root = false;
    std::string model_path;
};


