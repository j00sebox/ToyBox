#pragma once

#include <vector>

class VertexBuffer
{
public:
	VertexBuffer(const std::vector<float>& buffer);
	~VertexBuffer();

	void bind() const;
	void unbind() const;

private:
	unsigned int m_id;
};

class IndexBuffer
{
public:
	IndexBuffer(const std::vector<unsigned int>& buffer);
	~IndexBuffer();

	unsigned int get_count() const { return m_count; }

	void bind() const;
	void unbind() const;

private:
	unsigned int m_id;
	unsigned int m_count;
};