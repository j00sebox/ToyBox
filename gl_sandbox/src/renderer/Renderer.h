#pragma once

#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"

class Renderer
{
public:
	void draw(const VertexArray& va, const IndexBuffer& ib, const ShaderProgram& s);

};

