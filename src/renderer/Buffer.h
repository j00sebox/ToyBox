#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

// TODO: Think about combing into one class

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

enum class BufferType : int
{
    UNIFORM = 0,
    SHADER_STORAGE
};

class Buffer
{
public:
    explicit Buffer(unsigned int size, BufferType buffer_type);
    Buffer(Buffer&& ubo) noexcept;
    ~Buffer();

    void link(unsigned int binding_point) const;
    void bind() const;
    void unbind() const;

    template<typename T>
    void set_data(int offset, T&& data) const;

    void operator= (Buffer&& other_buffer);

private:
    unsigned int m_id;
    BufferType m_buffer_type;

    void set_data(int offset, uint64_t size, const void* data) const;
};

template<typename T>
void Buffer::set_data(int offset, T&& data) const
{
    uint64_t size = sizeof(T);
    set_data(offset, size, (void*)&data);
}