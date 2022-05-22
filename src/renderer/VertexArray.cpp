#include "pch.h"
#include "VertexArray.h"

#include "GLError.h"

#include <glad/glad.h>

int gl_get_type_size(GLenum type)
{
	switch (type)
	{
	case GL_FLOAT: return sizeof(float);
	case GL_INT: return sizeof(int);
	case GL_UNSIGNED_INT: return sizeof(unsigned int);
	default:
		break;
	}

	return 0;
}

VertexArray::VertexArray()
{
	GL_CALL(glGenVertexArrays(1, &m_id));
}

VertexArray::~VertexArray()
{
	GL_CALL(glDeleteVertexArrays(1, &m_id));
}

void VertexArray::set_layout(const VertexBuffer& vb, const BufferLayout& layout)
{
	bind();
	vb.bind();

	for (const auto& attrib : layout.get_layout())
	{
		GL_CALL(glEnableVertexAttribArray(attrib.id));
		GL_CALL(glVertexAttribPointer(attrib.id, attrib.num, attrib.type, attrib.normalized, layout.get_stride(), (void*)attrib.offset));
	}
}

void VertexArray::bind() const
{
	GL_CALL(glBindVertexArray(m_id));
}

void VertexArray::unbind() const
{
	GL_CALL(glBindVertexArray(0));
}

void VertexArray::operator= (VertexArray&& va) noexcept
{
	m_id = va.m_id;
	va.m_id = 0;
}

BufferLayout::BufferLayout(std::initializer_list<VertexAttribute> attribs)
	: m_layout(attribs)
{
	for (auto& a : m_layout)
	{
		calculate_stride_offset(a);
	}
}

void BufferLayout::add_attribute(VertexAttribute& attrib)
{
	calculate_stride_offset(attrib);
	m_layout.push_back(attrib);
}

void BufferLayout::calculate_stride_offset(VertexAttribute& attrib)
{
	m_stride += gl_get_type_size(attrib.type) * attrib.num;
	attrib.offset = m_offset;
	m_offset = m_stride;
}
