#pragma once

#include "Window.h"
#include "scene/Scene.h"

class Application
{
public:
	Application() : m_window(1920, 1080) { m_current_scene.reset(new Scene()); }
	void start();
	void switch_scene(const char* scene_path);

private:
	void display_menu();

	Window m_window;
	std::unique_ptr<Scene> m_current_scene;
	bool m_running = false;
};

