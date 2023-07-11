#pragma once

#include "Scene.h"
#include "SceneNode.h"

#include <queue>

class Inspector
{
public:
    Scene* scene = nullptr;
    void render();

private:
    void imguiRender(SceneNode& currentNode);
    void displayComponents();


    SceneNode* dragNode = nullptr;
    SceneNode* dropNode = nullptr;
    std::queue<SceneNode*> m_nodes_to_remove;
};

