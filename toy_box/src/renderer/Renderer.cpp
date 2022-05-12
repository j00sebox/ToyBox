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

	GL_CALL(glEnable(GL_DEPTH_TEST));
	GL_CALL(glDepthFunc(GL_LEQUAL));
	GL_CALL(glFrontFace(GL_CCW));

	GL_CALL(glClearColor(0.2f, 0.0f, 0.3f, 1.f));
}

void Renderer::draw_elements(unsigned count)
{
	GL_CALL(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr));
}

void Renderer::clear()
{
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}
