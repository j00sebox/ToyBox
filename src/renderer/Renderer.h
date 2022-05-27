#pragma once

#include "components/Fwd.h"

#include <mathz/Vector.h>

class Renderer
{
public:
	static void init(int width, int height);
	static void set_viewport(int width, int height);
	static void set_clear_colour(mathz::Vec4 colour);
	static void draw_elements(const Mesh&, const Material&);
	static void stencil(const Transform& stencil_transform, const Mesh&, const Material&);
	static void clear();
};

