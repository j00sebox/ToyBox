#include "pch.h"
#include "Buffer.h"

#include "GLError.h"

#include <glad/glad.h>

VertexBuffer::VertexBuffer()
{
	GL_CALL(glGenBuffers(1, &m_id));
}

VertexBuffer::~VertexBuffer()
{
	GL_CALL(glDeleteBuffers(1, &m_id));
}

void VertexBuffer::add_data(float* vertices, unsigned int size) const
{
	bind();
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW));
}

void VertexBuffer::bind() const
{
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_id));
}

void VertexBuffer::unbind() const
{
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
