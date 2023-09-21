#pragma once
#include "Scene.hpp"
#include "SceneNode.hpp"

#include <queue>

class Inspector
{
public:
    Inspector(Scene* scene) : m_scene(scene) {}
    void render();

private:
    Scene* m_scene;
    SceneNode* dragNode = nullptr;
    SceneNode* dropNode = nullptr;
    std::queue<SceneNode*> m_nodes_to_remove;

    void imguiRender(SceneNode* currentNode);
    void displayComponents() const;
};

