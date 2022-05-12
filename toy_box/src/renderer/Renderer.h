#pragma once

class Renderer
{
public:
	static void init(int width, int height);
	static void draw_elements(unsigned count);
	static void clear();
};

