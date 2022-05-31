#include "pch.h"
#include "Buffer.h"

#include "GLError.h"

#include <glad/glad.h>

VertexBuffer::VertexBuffer(const std::vector<float>& buffer)
{
	GL_CALL(glGenBuffers(1, &m_id));
	bind();
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW));
}

VertexBuffer::~VertexBuffer()
{
	GL_CALL(glDeleteBuffers(1, &m_id));
}

void VertexBuffer::bind() const
{
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, m_id));
}

void VertexBuffer::unbind() const
{
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

IndexBuffer::IndexBuffer(const std::vector<unsigned int>& buffer)
{
	m_count = buffer.size();

	GL_CALL(glGenBuffers(1, &m_id));
	bind();
	GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.size() * sizeof(unsigned int), buffer.data(), GL_STATIC_DRAW));
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

UniformBuffer::UniformBuffer(unsigned int size)
{
    GL_CALL(glGenBuffers(1, &m_id));
    bind();
    GL_CALL(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW));
}

UniformBuffer::UniformBuffer(UniformBuffer &&ubo) noexcept
{
    m_id = ubo.m_id;
    ubo.m_id = 0;
}

UniformBuffer::~UniformBuffer()
{
    GL_CALL(glDeleteBuffers(1, &m_id));
}

void UniformBuffer::set_data_scalar(unsigned int offset, void *data) const
{
    bind();
    GL_CALL(glBufferSubData(GL_UNIFORM_BUFFER, offset, 4, data));
    unbind();
}

void UniformBuffer::set_data_vec3(unsigned int offset, const mathz::Vec3& vec) const
{
    bind();
    float data[3] = {vec.x, vec.y, vec.z};
    GL_CALL(glBufferSubData(GL_UNIFORM_BUFFER, offset, 4 * 3, data));
    unbind();
}

void UniformBuffer::set_data_vec4(unsigned int offset, const mathz::Vec4& vec) const
{
    bind();
    float data[4] = {vec.x, vec.y, vec.z, vec.w};
    GL_CALL(glBufferSubData(GL_UNIFORM_BUFFER, offset, 4 * 4, data));
    unbind();
}

void UniformBuffer::set_data_mat4(unsigned int offset, const mathz::Mat4& mat) const
{
    bind();
    GL_CALL(glBufferSubData(GL_UNIFORM_BUFFER, offset, 64, &mat.mat[0][0]));
    unbind();
}

void UniformBuffer::link(unsigned int binding_point) const
{
    GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, m_id));
}

void UniformBuffer::bind() const
{
    GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, m_id));
}

void UniformBuffer::unbind() const
{
    GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}