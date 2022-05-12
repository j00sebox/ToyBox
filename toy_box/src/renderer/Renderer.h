#pragma once

class Renderer
{
public:
	static void init(int width, int height);
	static void draw_elements(unsigned int count);
	static void stencil(unsigned int count);
	static void clear();
};

