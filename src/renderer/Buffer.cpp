#include "pch.h"
#include "Buffer.h"
#include "GLError.h"

#include <glad/glad.h>

VertexBuffer::VertexBuffer()
{
    GL_CALL(glGenBuffers(1, &m_id));
}

VertexBuffer::VertexBuffer(VertexBuffer&& vb)
{
    m_id = vb.m_id;
    vb.m_id = 0;
}

VertexBuffer::VertexBuffer(const std::vector<float>& buffer)
{
	GL_CALL(glGenBuffers(1, &m_id));
    set_data(buffer);
}

VertexBuffer::~VertexBuffer()
{
	GL_CALL(glDeleteBuffers(1, &m_id));
}

void VertexBuffer::set_data(const std::vector<float> &buffer)
{
    bind();
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW));
}

void VertexBuffer::bind() const
{
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_id));
}

void VertexBuffer::unbind() const
{
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBuffer::operator=(VertexBuffer &&vb)
{
    m_id = vb.m_id;
    vb.m_id = 0;
}

IndexBuffer::IndexBuffer()
{
    GL_CALL(glGenBuffers(1, &m_id));
}

IndexBuffer::IndexBuffer(IndexBuffer&& ib)
{
    m_id = ib.m_id;
    ib.m_id = 0;
    m_count = ib.m_count;
}

IndexBuffer::IndexBuffer(const std::vector<unsigned int>& buffer)
{
	m_count = buffer.size();

	GL_CALL(glGenBuffers(1, &m_id));
    set_data(buffer);
}

IndexBuffer::~IndexBuffer()
{
	GL_CALL(glDeleteBuffers(1, &m_id));
}

void IndexBuffer::set_data(const std::vector<unsigned int> &buffer)
{
    bind();
    GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size() * sizeof(unsigned int), buffer.data(), GL_STATIC_DRAW));
}

void IndexBuffer::bind() const
{
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
}

void IndexBuffer::unbind() const
{
	GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void IndexBuffer::operator=(IndexBuffer&& ib)
{
    m_id = ib.m_id;
    ib.m_id = 0;
    m_count = ib.m_count;
}

GLenum convert_buffer_type(BufferType type)
{
    switch (type)
    {
        case BufferType::UNIFORM :          return GL_UNIFORM_BUFFER;
        case BufferType::SHADER_STORAGE :   return GL_SHADER_STORAGE_BUFFER;
    }
}

Buffer::Buffer(unsigned int size, BufferType buffer_type)
{
    m_buffer_type = buffer_type;
    GL_CALL(glGenBuffers(1, &m_id));
    bind();
    GL_CALL(glBufferData(convert_buffer_type(m_buffer_type), size, nullptr, GL_STATIC_DRAW));
}

Buffer::Buffer(Buffer&& buff) noexcept
{
    m_id = buff.m_id;
    m_buffer_type = buff.m_buffer_type;
    buff.m_id = 0;
}

Buffer::~Buffer()
{
    GL_CALL(glDeleteBuffers(1, &m_id));
}

void Buffer::link(unsigned int binding_point) const
{
    GL_CALL(glBindBufferBase(convert_buffer_type(m_buffer_type), binding_point, m_id));
}

void Buffer::bind() const
{
    GL_CALL(glBindBuffer(convert_buffer_type(m_buffer_type), m_id));
}

void Buffer::unbind() const
{
    GL_CALL(glBindBuffer(convert_buffer_type(m_buffer_type), 0));
}

void Buffer::set_data(int offset, uint64_t size, const void* data) const
{
    bind();
    GL_CALL(glNamedBufferSubData(m_id, offset, size, data))
    unbind();
}

void Buffer::operator=(Buffer&& other_buffer)
{
    m_id = other_buffer.m_id;
    m_buffer_type = other_buffer.m_buffer_type;
    other_buffer.m_id = 0;
}




