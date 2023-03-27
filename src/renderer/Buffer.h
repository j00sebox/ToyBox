#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

class VertexBuffer
{
public:
    VertexBuffer();
    VertexBuffer(VertexBuffer&& vb);
	explicit VertexBuffer(const std::vector<float>& buffer);
	~VertexBuffer();

    void set_data(const std::vector<float>& buffer);

	void bind() const;
	void unbind() const;

    void operator= (VertexBuffer&& vb);

private:
	unsigned int m_id;
};

class IndexBuffer
{
public:
    IndexBuffer();
    IndexBuffer(IndexBuffer&& ib);
	explicit IndexBuffer(const std::vector<unsigned int>& buffer);
	~IndexBuffer();

    void set_data(const std::vector<unsigned int>& buffer);

	[[nodiscard]] unsigned int get_count() const { return m_count; }

	void bind() const;
	void unbind() const;

    void operator= (IndexBuffer&& ib);

private:
	unsigned int m_id;
	unsigned int m_count;
};

class UniformBuffer
{
public:
    explicit UniformBuffer(unsigned int size);
    UniformBuffer(UniformBuffer&& ubo) noexcept;
    ~UniformBuffer();

    void set_data_scalar_i(unsigned int offset, int data) const;
    void set_data_scalar_f(unsigned int offset, float data) const;
    void set_data_vec3(unsigned int offset, const glm::vec3& vec) const;
    void set_data_vec4(unsigned int offset, const glm::vec4& vec) const;
    void set_data_mat4(unsigned int offset, const glm::mat4& mat) const;

    void link(unsigned int binding_point) const;
    void bind() const;
    void unbind() const;

private:
    unsigned int m_id;
};