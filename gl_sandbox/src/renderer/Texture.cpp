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
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_id));

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_data));

	stbi_image_free(m_data);
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

CubeMap::CubeMap(const std::string& dir)
{
	m_faces[0] = dir + "right.jpg"; 
	m_faces[1] = dir + "left.jpg";
	m_faces[2] = dir + "top.jpg";
	m_faces[3] = dir + "bottom.jpg";
	m_faces[4] = dir + "front.jpg";
	m_faces[5] = dir + "back.jpg";

	GL_CALL(glGenTextures(1, &m_id));
	GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, m_id));
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// These are very important to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (int i = 0; i < 6; ++i)
	{
		int width, height, num_channels;
		unsigned char* data = stbi_load(m_faces[0].c_str(), &width, &height, &num_channels, 4);

		if (!data)
			ASSERT(false);

		GL_CALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));

		stbi_image_free(data);
	}
	
}

CubeMap::~CubeMap()
{
	GL_CALL(glDeleteTextures(1, &m_id));
}

void CubeMap::bind(int slot) const
{
	GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, m_id));
}

void CubeMap::unbind() const
{
	GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}
