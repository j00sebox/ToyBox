#pragma once

#include "Window.h"
#include "Scene.h"

class Application
{
public:
	Application() : m_window(1280, 960) {}
	void start();

private:
	Window m_window;
	Scene m_current_scene;
	bool m_running = false;
};

