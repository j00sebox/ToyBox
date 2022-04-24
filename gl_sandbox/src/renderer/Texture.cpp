#include "pch.h"
#include "Texture.h"

#include "GLError.h"

#include <glad/glad.h>
#include <stb_image.h>

Texture2D::Texture2D(const std::string& file_name)
{
	stbi_set_flip_vertically_on_load(1);
	m_data = stbi_load(file_name.c_str(), &m_width, &m_width, &m_bpp, 4);

	GL_CALL(glGenTextures(1, &m_id));
	glBindTexture(GL_TEXTURE_2D, m_id);

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data));
}

Texture2D::~Texture2D()
{
	GL_CALL(glDeleteTextures(1, &m_id));
}

void Texture2D::bind(int slot /* = 0 */) const
{
	GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_id));
}

void Texture2D::unbind() const
{
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}
