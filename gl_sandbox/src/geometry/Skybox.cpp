#include "pch.h"
#include "Skybox.h"

#include "GLError.h"

#include <glad/glad.h>

Skybox::Skybox()
{
	float skybox_verts[] =
	{
		//   Coordinates
		-1.0f, -1.0f,  1.0f,	//        7--------6
		 1.0f, -1.0f,  1.0f,	//       /|       /|
		 1.0f, -1.0f, -1.0f,	//      4--------5 |
		-1.0f, -1.0f, -1.0f,	//      | |      | | 
		-1.0f,  1.0f,  1.0f,	//      | 3------|-2
		 1.0f,  1.0f,  1.0f,	//      |/       |/
		 1.0f,  1.0f, -1.0f,	//      0--------1
		-1.0f,  1.0f, -1.0f
	};

	unsigned int skybox_indices[] =
	{
		// Right
		1, 2, 6,
		6, 5, 1,
		// Left
		0, 4, 7,
		7, 3, 0,
		// Top
		4, 5, 6,
		6, 7, 4,
		// Bottom
		0, 3, 2,
		2, 1, 0,
		// Back
		0, 1, 5,
		5, 4, 0,
		// Front
		3, 7, 6,
		6, 2, 3
	};

	m_skybox_va.reset(new VertexArray());

	m_skybox_vb = VertexBuffer();
	m_skybox_vb.add_data(skybox_verts, sizeof(skybox_verts));

	BufferLayout sb_layout = { {0, 3, GL_FLOAT, false} };

	m_skybox_va->set_layout(m_skybox_vb, sb_layout);

	m_skybox_ib.reset(new IndexBuffer(skybox_indices, sizeof(skybox_indices)));
}

void Skybox::draw() const
{
	GL_CALL(glDepthMask(GL_FALSE));
	m_skybox_va->bind();
	m_skybox_ib->bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, m_skybox_ib->get_count(), GL_UNSIGNED_INT, nullptr));
	GL_CALL(glDepthMask(GL_TRUE));
}
