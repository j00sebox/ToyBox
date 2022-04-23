#include "pch.h"
#include "Renderer.h"

#include "GLError.h"

#include <glad/glad.h>

void Renderer::draw(const VertexArray& va, const IndexBuffer& ib, const ShaderProgram& s)
{
	va.bind();
	ib.bind();
	s.bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, ib.get_count(), GL_UNSIGNED_INT, nullptr));
}
