#include "pch.h"
#include "Skybox.h"
#include "Buffer.h"
#include "GLError.h"

#include <glad/glad.h>

Skybox::Skybox(const std::string& texture_path)
	: m_skybox_texture(texture_path), m_path(texture_path)
{
    m_skybox_shader = ShaderLib::get("skybox");

	std::vector<float> skybox_verts =
	{
		-1.0f, -1.0f,  1.0f,	//        7--------6
		 1.0f, -1.0f,  1.0f,	//       /|       /|
		 1.0f, -1.0f, -1.0f,	//      4--------5 |
		-1.0f, -1.0f, -1.0f,	//      | |      | | 
		-1.0f,  1.0f,  1.0f,	//      | 3------|-2
		 1.0f,  1.0f,  1.0f,	//      |/       |/
		 1.0f,  1.0f, -1.0f,	//      0--------1
		-1.0f,  1.0f, -1.0f
	};

	std::vector<unsigned int> skybox_indices =
	{
		// right
		1, 2, 6,
		6, 5, 1,

		// left
		0, 4, 7,
		7, 3, 0,

		// top
		4, 5, 6,
		6, 7, 4,

		// bottom
		0, 3, 2,
		2, 1, 0,

		// back
		0, 1, 5,
		5, 4, 0,

		// front
		3, 7, 6,
		6, 2, 3
	};

	m_skybox_va.bind();

	VertexBuffer skybox_vb(skybox_verts);
	IndexBuffer skybox_ib(skybox_indices);

	m_indices_count = skybox_ib.get_count();

	BufferLayout sb_layout = { {0, 3, GL_FLOAT, false} };

	m_skybox_va.set_layout(skybox_vb, sb_layout);

	m_skybox_va.unbind();
	skybox_ib.unbind();
	skybox_vb.unbind();
}

Skybox::Skybox(Skybox&& sb) noexcept
	: m_skybox_texture(std::move(sb.m_skybox_texture))
{
	m_indices_count = sb.m_indices_count;
	m_skybox_va = std::move(sb.m_skybox_va);
    m_skybox_shader = sb.m_skybox_shader;
	m_path = std::move(sb.m_path);
}

void Skybox::draw() const
{
    GL_CALL(glDisable(GL_CULL_FACE));
	GL_CALL(glDepthMask(GL_FALSE));
	m_skybox_va.bind();
	m_skybox_texture.bind(0);
	m_skybox_shader->bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, m_indices_count, GL_UNSIGNED_INT, nullptr));
	GL_CALL(glDepthMask(GL_TRUE));
	m_skybox_va.unbind();
	m_skybox_texture.unbind();
	m_skybox_shader->unbind();
    //GL_CALL(glEnable(GL_CULL_FACE));
}

void Skybox::bind() const
{
    m_skybox_va.bind();
    m_skybox_texture.bind(0);
    m_skybox_shader->bind();
}

void Skybox::unbind() const
{
    m_skybox_va.unbind();
    m_skybox_texture.unbind();
    m_skybox_shader->unbind();
}


void Skybox::operator=(Skybox&& sb) noexcept
{
	m_indices_count = sb.m_indices_count;
	m_skybox_va = std::move(sb.m_skybox_va);
	m_skybox_texture = std::move(sb.m_skybox_texture);
	m_skybox_shader = sb.m_skybox_shader;
}


