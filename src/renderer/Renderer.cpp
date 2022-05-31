#include "pch.h"
#include "Renderer.h"

#include "GLError.h"
#include "Shader.h"
#include "components/Mesh.h"
#include "components/Material.h"
#include "components/Transform.h"
#include "events/EventList.h"

#include <glad/glad.h>

void Renderer::init(int width, int height)
{
	set_viewport(width, height);

    glEnable(GL_CULL_FACE);
//	GL_CALL(glEnable(GL_BLEND));
//	GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL_CALL(glEnable(GL_STENCIL_TEST));
	GL_CALL(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE)); // replace all the stencil values when drawing original object
	GL_CALL(glEnable(GL_DEPTH_TEST));
	GL_CALL(glDepthFunc(GL_LEQUAL));
}

void Renderer::set_viewport(int width, int height)
{
	GL_CALL(glViewport(0, 0, width, height));
}

void Renderer::set_clear_colour(mathz::Vec4 colour)
{
	GL_CALL(glClearColor(colour.x, colour.y, colour.z, colour.w));
}

void Renderer::draw_elements(const Mesh& mesh, const Material& material)
{
	material.bind();
	mesh.bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, mesh.get_index_count(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::stencil(const Transform& stencil_transform, const Mesh& mesh, const Material& material)
{
	GL_CALL(glStencilFunc(GL_ALWAYS, 1, 0xFF)); // make all the fragments of the object have a stencil of 1
	GL_CALL(glStencilMask(0xFF)); // any stencil value can be written to
	
	material.bind();
	mesh.bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, mesh.get_index_count(), GL_UNSIGNED_INT, nullptr));

    ShaderLib::get("flat_colour")->set_uniform_mat4f("u_model", stencil_transform.get_transform());
    ShaderLib::get("flat_colour")->set_uniform_4f("u_flat_colour", {1.f, 1.f, 0.f, 1.f});
	ShaderLib::get("flat_colour")->bind();
	GL_CALL(glStencilFunc(GL_NOTEQUAL, 1, 0xFF)); // now all fragments not apart of the original object are written
	GL_CALL(glStencilMask(0x00)); // disable writing to stencil buffer
	GL_CALL(glDisable(GL_DEPTH_TEST));

	GL_CALL(glDrawElements(GL_TRIANGLES, mesh.get_index_count(), GL_UNSIGNED_INT, nullptr));
	
	// set back to normal for other objects
	GL_CALL(glStencilMask(0xFF));
	GL_CALL(glStencilFunc(GL_ALWAYS, 0, 0xFF));
	GL_CALL(glEnable(GL_DEPTH_TEST));
}

void Renderer::clear()
{
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}
