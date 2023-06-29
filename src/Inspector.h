#pragma once

#include "SceneNode.h"

#include <queue>

class Inspector
{
public:
    void render(SceneNode& root);
    SceneNode* getSelectedNode() { return selectedNode; }

private:
    void imguiRender(SceneNode& currentNode);
    void displayComponents();

    SceneNode* selectedNode = nullptr;
    SceneNode* dragNode = nullptr;
    SceneNode* dropNode = nullptr;
    std::queue<SceneNode*> m_nodes_to_remove;
};

