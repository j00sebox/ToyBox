#pragma once

#include "components/Fwd.h"

class Renderer
{
public:
	static void init(int width, int height);
	static void set_viewport(int width, int height);
	static void draw_elements(const Mesh&, const Material&);
	static void stencil(const Mesh&, const Material&);
	static void clear();
};

