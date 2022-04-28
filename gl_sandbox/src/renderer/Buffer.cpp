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

void VertexBuffer::add_data(const float* vertices, const unsigned int size) const
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

IndexBuffer::IndexBuffer(const unsigned int* indices, const unsigned int size)
{
	m_count = size / sizeof(unsigned int);

	GL_CALL(glGenBuffers(1, &m_id));
	bind();
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer()
{
	GL_CALL(glDeleteBuffers(1, &m_id));
}

void IndexBuffer::bind() const
{
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
}

void IndexBuffer::unbind() const
{
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
