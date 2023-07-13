#include "pch.h"
#include "Buffer.h"
#include "GLError.h"
#include "Log.h"

#include <glad/glad.h>

GLenum convert_buffer_type(BufferType type)
{
    switch (type)
    {
        case BufferType::VERTEX :           return GL_ARRAY_BUFFER;
        case BufferType::INDEX :            return GL_ELEMENT_ARRAY_BUFFER;
        case BufferType::UNIFORM :          return GL_UNIFORM_BUFFER;
        case BufferType::SHADER_STORAGE :   return GL_SHADER_STORAGE_BUFFER;
        default:                            return GL_NONE;
    }
}

Buffer::Buffer(BufferType buffer_type)
{
    m_buffer_type = buffer_type;
    GL_CALL(glGenBuffers(1, &m_id));
}

Buffer::Buffer(uint64_t size, BufferType buffer_type)
{
    m_buffer_type = buffer_type;
    GL_CALL(glGenBuffers(1, &m_id));
    allocate_memory(size);
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

void Buffer::allocate_memory(uint64_t size) const
{
    bind();
    GL_CALL(glBufferData(convert_buffer_type(m_buffer_type), size, nullptr, GL_STATIC_DRAW));
    unbind();
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

Buffer& Buffer::operator=(Buffer&& other_buffer) noexcept
{
    m_id = other_buffer.m_id;
    m_buffer_type = other_buffer.m_buffer_type;
    other_buffer.m_id = 0;
    return *this;
}




