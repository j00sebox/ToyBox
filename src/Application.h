#pragma once

#include "Window.h"
#include "Scene.h"

class Application
{
public:
	Application() : m_window(1920, 1080) { m_current_scene = std::make_unique<Scene>(); }
	void start();
	void switch_scene(const char* scene_path);

private:
	void display_dockspace();
	void display_menu();
	void display_fps();
	
	Window m_window;
	std::unique_ptr<Scene> m_current_scene;
	bool m_running = false;
	bool m_show_dock_space = true;
};

