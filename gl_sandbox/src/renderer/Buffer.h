#pragma once

class VertexBuffer
{
public:
	VertexBuffer();
	~VertexBuffer();

	void add_data(const float* vertices, const unsigned int size) const;

	void bind() const;
	void unbind() const;

private:
	unsigned int m_id;
};

class IndexBuffer
{
public:
	IndexBuffer(const unsigned int* indices, const unsigned int size);
	~IndexBuffer();

	unsigned int get_count() const { return m_count; }

	void bind() const;
	void unbind() const;

private:
	unsigned int m_id;
	unsigned int m_count;
};