#pragma once

#include "Buffer.h"

struct VertexAttribute
{
	unsigned id;
	int num;
	unsigned type;
	bool normalized;
	int offset = 0;
};

class BufferLayout
{
public:
	BufferLayout() = default;
	BufferLayout(std::initializer_list<VertexAttribute> attribs);

	void add_attribute(VertexAttribute& attrib);
	void calculate_stride_offset(VertexAttribute& attrib);

	[[nodiscard]] int get_stride() const { return m_stride; }
	[[nodiscard]] const std::vector<VertexAttribute>& get_layout() const { return m_layout; }

private:
	int m_stride = 0;
	int m_offset = 0;
	std::vector<VertexAttribute> m_layout;
};

class VertexArray
{
public:
	VertexArray();
    VertexArray(VertexArray&& va) noexcept;
	~VertexArray();

    void set_layout(const BufferLayout& layout) const;

	void bind() const;
	void unbind() const;

	void operator= (VertexArray&& va) noexcept;

private:
	unsigned int m_id;
};

