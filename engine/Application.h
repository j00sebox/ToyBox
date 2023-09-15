#pragma once

#include "Window.h"
#include "Scene.h"
#include "editor/Inspector.h"

class Application
{
public:
	Application()
        : m_window(1920, 1080, 600, 600)
    {
        currentScene = new Scene(&m_window);
        inspector.scene = currentScene;
    }
	void start();
	void switch_scene(const char* scene_path);
    void shutdown();

private:
	void display_dockspace();
	void display_menu();
	static void display_fps();

    Scene* currentScene;
    Window m_window;
    Inspector inspector;

	bool m_running = false;
	bool m_show_dock_space = true;
};

