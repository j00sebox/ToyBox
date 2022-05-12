#include "pch.h"
#include "Renderer.h"

#include "GLError.h"
#include "Texture.h"
#include "Shader.h"
#include "VertexArray.h"

#include "events/EventList.h"

#include "mathz/Quaternion.h";

#include <glad/glad.h>

void Renderer::init(int width, int height)
{
	GL_CALL(glViewport(0, 0, width, height));

	GL_CALL(glEnable(GL_STENCIL_TEST));
	GL_CALL(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE)); // replace all the stencil values when drawing original object
	GL_CALL(glEnable(GL_DEPTH_TEST));
	GL_CALL(glDepthFunc(GL_LEQUAL));
	GL_CALL(glFrontFace(GL_CCW));

	GL_CALL(glClearColor(0.2f, 0.0f, 0.3f, 1.f));

	ShaderLib::add("flat_colour", ShaderProgram(
		Shader("resources/shaders/flat_colour/flat_colour_vertex.shader", ShaderType::Vertex),
		Shader("resources/shaders/flat_colour/flat_colour_fragment.shader", ShaderType::Fragment)
	));
}

void Renderer::draw_elements(unsigned int count)
{
	GL_CALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr));
}

void Renderer::stencil(unsigned int count)
{
	GL_CALL(glStencilFunc(GL_ALWAYS, 1, 0xFF)); // make all the fragments of the object have a stencil of 1
	GL_CALL(glStencilMask(0xFF)); // any stencil value can be written to
	
	GL_CALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr));

	ShaderLib::get("flat_colour")->bind();
	GL_CALL(glStencilFunc(GL_NOTEQUAL, 1, 0xFF)); // now all fragments not apart of the original object are written
	GL_CALL(glStencilMask(0x00)); // disable writing to stencil buffer
	GL_CALL(glDisable(GL_DEPTH_TEST));

	GL_CALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr));
	
	// set back to normal for other objects
	GL_CALL(glStencilMask(0xFF));
	GL_CALL(glStencilFunc(GL_ALWAYS, 1, 0xFF));
	GL_CALL(glEnable(GL_DEPTH_TEST));
}

void Renderer::clear()
{
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}
