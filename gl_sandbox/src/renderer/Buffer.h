#pragma once

class VertexBuffer
{
public:
	VertexBuffer();
	~VertexBuffer();

	void add_data(float* vertices, unsigned int size) const;

	void bind() const;
	void unbind() const;

private:
	unsigned int m_id;
};