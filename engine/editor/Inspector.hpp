#pragma once
#include "Scene.hpp"
#include "SceneNode.hpp"

#include <queue>

struct Inspector
{
    void display(Scene* scene);

private:
    SceneNode* drag_node = nullptr;
    SceneNode* drop_node = nullptr;
    std::queue<SceneNode*> nodes_to_remove;

    void display_node(Scene* scene, SceneNode* currentNode);
    void display_components(SceneNode* selected_node) const;
};

