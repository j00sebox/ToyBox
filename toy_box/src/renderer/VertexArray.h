#pragma once

#include "Buffer.h"

struct VertexAttribute
{
	unsigned int id;
	int num;
	unsigned int type;
	bool normalized;
	int offset = 0;
};

class BufferLayout
{
public:
	BufferLayout() {}

	BufferLayout(std::initializer_list<VertexAttribute> attribs);

	void add_attribute(VertexAttribute& attrib);
	void calculate_stride_offset(VertexAttribute& attrib);

	int get_stride() const { return m_stride; }
	const std::vector<VertexAttribute>& get_layout() const { return m_layout; }

private:
	int m_stride = 0;
	int m_offset = 0;
	std::vector<VertexAttribute> m_layout;
};

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void set_layout(const VertexBuffer& vb, const BufferLayout& layout);

	void bind() const;
	void unbind() const;

	void operator= (VertexArray&& va) noexcept;

private:
	unsigned int m_id;
};

