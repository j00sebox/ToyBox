#pragma once

#include "components/Fwd.h"
#include "Transform.h"

#include <mathz/Vector.h>
#include <glm/vec4.hpp>

enum class RenderCommand
{
    ElementDraw = 0,
    Stencil
};

struct RenderObject
{
    RenderCommand render_command;
    Transform transform;
    Mesh* mesh;
    Material* material;
};

class Renderer
{
public:
	static void init(int width, int height);
	static void set_viewport(int width, int height);
	static void set_clear_colour(glm::vec4 colour);
	static void draw_elements(const Transform& transform, const Mesh&, const Material&);
	static void stencil(const Transform& stencil_transform, const Mesh&, const Material&);
    static void shadow_pass(const std::vector<RenderObject>& render_list);
    static void render_pass(const std::vector<RenderObject>& render_list);
	static void clear();
};

