#include "pch.h"
#include "Buffer.h"

#include <glad/glad.h>

VertexBuffer::VertexBuffer()
{
	glGenBuffers(1, &m_id);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_id);
}

void VertexBuffer::add_data(float* vertices, unsigned int size) const
{
	bind();
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

void VertexBuffer::bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBuffer::unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
