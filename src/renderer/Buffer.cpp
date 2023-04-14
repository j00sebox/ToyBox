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

void UniformBuffer::set_data_scalar_i(unsigned int offset, int data) const
{
    bind();
    GL_CALL(glBufferSubData(GL_UNIFORM_BUFFER, offset, 4, (void*)&data));
    unbind();
}

void UniformBuffer::set_data_scalar_f(unsigned int offset, float data) const
{
    bind();
    GL_CALL(glBufferSubData(GL_UNIFORM_BUFFER, offset, 4, (void*)&data));
    unbind();
}

void UniformBuffer::set_data_vec3(unsigned int offset, const glm::vec3& vec) const
{
    bind();
    float data[3] = {vec.x, vec.y, vec.z};
    GL_CALL(glBufferSubData(GL_UNIFORM_BUFFER, offset, 4 * 3, data));
    unbind();
}

void UniformBuffer::set_data_vec4(unsigned int offset, const glm::vec4& vec) const
{
    bind();
    float data[4] = {vec.x, vec.y, vec.z, vec.w};
    GL_CALL(glBufferSubData(GL_UNIFORM_BUFFER, offset, 4 * 4, data));
    unbind();
}

void UniformBuffer::set_data_mat4(unsigned int offset, const glm::mat4& mat) const
{
    bind();
    GL_CALL(glBufferSubData(GL_UNIFORM_BUFFER, offset, 64, &mat[0][0]));
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

ShaderStorageBuffer::ShaderStorageBuffer(unsigned int size)
{
    GL_CALL(glGenBuffers(1, &m_id));
    bind();
    GL_CALL(glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_STATIC_DRAW));
}

ShaderStorageBuffer::ShaderStorageBuffer(ShaderStorageBuffer &&ssb) noexcept
{
    m_id = ssb.m_id;
    ssb.m_id = 0;
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
    GL_CALL(glDeleteBuffers(1, &m_id));
}


void ShaderStorageBuffer::set_data_scalar_i(unsigned int offset, int data) const
{
    bind();
    GL_CALL(glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, 4, (void*)&data));
    unbind();
}

void ShaderStorageBuffer::set_data_scalar_f(unsigned int offset, float data) const
{
    bind();
    GL_CALL(glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, 4, (void*)&data));
    unbind();
}

void ShaderStorageBuffer::set_data_vec3(unsigned int offset, const glm::vec3& vec) const
{
    bind();
    float data[3] = {vec.x, vec.y, vec.z};
    GL_CALL(glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, 4 * 3, data));
    unbind();
}

void ShaderStorageBuffer::set_data_vec4(unsigned int offset, const glm::vec4& vec) const
{
    bind();
    float data[4] = {vec.x, vec.y, vec.z, vec.w};
    GL_CALL(glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, 4 * 4, data));
    unbind();
}

void ShaderStorageBuffer::set_data_mat4(unsigned int offset, const glm::mat4& mat) const
{
    bind();
    GL_CALL(glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, 64, &mat[0][0]));
    unbind();
}

void ShaderStorageBuffer::link(unsigned int binding_point) const
{
    GL_CALL(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, m_id));
}

void ShaderStorageBuffer::bind() const
{
    GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id));
}

void ShaderStorageBuffer::unbind() const
{
    GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}



