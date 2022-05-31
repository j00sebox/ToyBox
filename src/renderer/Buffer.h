#pragma once

#include <vector>
#include <mathz/Matrix.h>

class VertexBuffer
{
public:
	explicit VertexBuffer(const std::vector<float>& buffer);
	~VertexBuffer();

	void bind() const;
	void unbind() const;

private:
	unsigned int m_id;
};

class IndexBuffer
{
public:
	explicit IndexBuffer(const std::vector<unsigned int>& buffer);
	~IndexBuffer();

	[[nodiscard]] unsigned int get_count() const { return m_count; }

	void bind() const;
	void unbind() const;

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

    void set_data_scalar(unsigned int offset, void* data) const;
    void set_data_vec3(unsigned int offset, const mathz::Vec3& vec) const;
    void set_data_vec4(unsigned int offset, const mathz::Vec4& vec) const;
    void set_data_mat4(unsigned int offset, const mathz::Mat4& mat) const;

    void link(unsigned int binding_point) const;
    void bind() const;
    void unbind() const;

private:
    unsigned int m_id;
};