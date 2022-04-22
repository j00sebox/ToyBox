#pragma once

class VertexBuffer
{
public:
	VertexBuffer();
	~VertexBuffer();

	void bind() const;
	void unbind() const;
};