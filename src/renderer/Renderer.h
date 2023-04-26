#pragma once

#include "Transform.h"
#include "MeshComponent.h"
#include "MaterialComponent.h"
#include "Skybox.h"
#include "Mesh.h"
#include "Material.h"

#include <list>
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
    MeshComponent mesh;
    MaterialComponent material;
    unsigned int instances = 1;
};

class Renderer
{
public:
	static void init(int width, int height);
	static void set_viewport(int width, int height);
	static void set_clear_colour(glm::vec4 colour);
	static void draw_elements(const Transform& transform, const Mesh&, const Material&);
    static void draw_elements_instanced(unsigned int instances, const Mesh&, const Material&);
    static void draw_skybox(const Skybox& skybox);
	static void stencil(const Transform& stencil_transform, const Mesh&, const Material&);
    static void shadow_pass(const std::vector<RenderObject>& render_list, unsigned int shadow_width = 2048, unsigned int shadow_height = 2048, bool using_cubemap = false);
    static void render_pass(const std::vector<RenderObject>& render_list);
	static void clear();
};

