#pragma once

#include "components/Fwd.h"
#include "Transform.h"
#include "Skybox.h"

#include <glm/vec4.hpp>

enum class RenderCommand
{
    ElementDraw = 0,
    InstancedElementDraw,
    Stencil
};

struct RenderObject
{
    RenderCommand render_command;
    Transform transform;
    MeshObject* mesh;
    Material* material;
    unsigned int instances = 1;
};

class Renderer
{
public:
	static void init(int width, int height);
	static void set_viewport(int width, int height);
	static void set_clear_colour(glm::vec4 colour);
	static void draw_elements(const Transform& transform, const MeshObject&, const Material&);
    static void draw_elements_instanced(unsigned int instances, const MeshObject&, const Material&);
    static void draw_skybox(const Skybox& skybox);
	static void stencil(const Transform& stencil_transform, const MeshObject&, const Material&);
    static void shadow_pass(const std::vector<RenderObject>& render_list, bool using_cubemap = false);
    static void render_pass(const std::vector<RenderObject>& render_list);
	static void clear();

	// FIXME
	static unsigned int shadow_map;
    static unsigned int shadow_cube_map;
};

