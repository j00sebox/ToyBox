#pragma once

#include "Window.h"

class Application
{
public:
	Application() : m_window(1280, 960) {}
	void start();

private:
	Window m_window;
	bool m_running = false;
};

