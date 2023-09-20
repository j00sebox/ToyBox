#pragma once

#include "Scene.hpp"
#include "SceneNode.hpp"

#include <queue>

class Inspector
{
public:
    Scene* scene = nullptr;
    void render();

private:
    void imguiRender(SceneNodePtr& currentNode);
    void displayComponents();


    SceneNodePtr dragNode = nullptr;
    SceneNodePtr dropNode = nullptr;
    std::queue<SceneNodePtr> m_nodes_to_remove;
};

