#include "pch.h"
#include "Texture.h"

#include "GLError.h"

#include <glad/glad.h>
#include <stb_image.h>

Texture2D::Texture2D(const std::string& file_name)
{
	stbi_set_flip_vertically_on_load(0);
	m_data = stbi_load(file_name.c_str(), &m_width, &m_height, &m_colour_channels, 0);

	GL_CALL(glGenTextures(1, &m_id));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_id));

	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

	unsigned int pixel_format;

	switch (m_colour_channels)
	{
	case 4:
		pixel_format = GL_RGBA;
		break;
	case 3:
		pixel_format = GL_RGB;
		break;
	default:
		pixel_format = GL_RGBA;
		break;
	}

	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, pixel_format, GL_UNSIGNED_BYTE, m_data));

	GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

	stbi_image_free(m_data);
}

Texture2D::Texture2D(Texture2D&& t) noexcept
{
	move_members(std::move(t));
}

Texture2D::~Texture2D()
{
	GL_CALL(glDeleteTextures(1, &m_id));
}

void Texture2D::bind(unsigned int slot /* = 0 */) const
{
	GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_id));
}

void Texture2D::unbind() const
{
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void Texture2D::operator=(Texture2D&& t) noexcept
{
	move_members(std::move(t));
}

void Texture2D::move_members(Texture2D&& t) noexcept
{
	m_id = t.m_id;
	t.m_id = 0;
	m_width = t.m_width;
	m_height = t.m_height;
	m_colour_channels = t.m_colour_channels;
	m_data = t.m_data;
	t.m_data = nullptr;
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
	GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	
	GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CALL(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

	GL_CALL(glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS));

	stbi_set_flip_vertically_on_load(1);

	for (int i = 0; i < 6; ++i)
	{
		int width, height, num_channels;
		unsigned char* data = stbi_load(m_faces[i].c_str(), &width, &height, &num_channels, 4);
		stbi_set_flip_vertically_on_load(0);

		if (!data)
			fatal("Could not load image: %s! No data!", m_faces[i]);

		GL_CALL(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));

		stbi_image_free(data);
	}
	
}

CubeMap::CubeMap(CubeMap&& cb) noexcept
{
	m_id = cb.m_id;
	cb.m_id = 0;
}

CubeMap::~CubeMap()
{
	GL_CALL(glDeleteTextures(1, &m_id));
}

void CubeMap::bind(unsigned int slot) const
{
	GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
	GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, m_id));
}

void CubeMap::unbind() const
{
	GL_CALL(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

void CubeMap::operator=(CubeMap&& cb) noexcept
{
	m_id = cb.m_id;
	cb.m_id = 0;
}
