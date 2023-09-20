#include "pch.h"
//#include "Skybox.h"
//#include "Buffer.h"

// #include <glad/glad.h>

//Skybox::Skybox(const std::string& texture_path, ImageFormat fmt)
//	: m_skybox_texture(texture_path, fmt), m_path(texture_path), m_img_fmt(fmt)
//{
//    m_skybox_shader = ShaderTable::get("skybox");
//
//	const std::vector<float> skybox_verts =
//	{
//		-1.0f, -1.0f,  1.0f,	//        7--------6
//		 1.0f, -1.0f,  1.0f,	//       /|       /|
//		 1.0f, -1.0f, -1.0f,	//      4--------5 |
//		-1.0f, -1.0f, -1.0f,	//      | |      | |
//		-1.0f,  1.0f,  1.0f,	//      | 3------|-2
//		 1.0f,  1.0f,  1.0f,	//      |/       |/
//		 1.0f,  1.0f, -1.0f,	//      0--------1
//		-1.0f,  1.0f, -1.0f
//	};
//
//	const std::vector<unsigned int> skybox_indices =
//	{
//		// right
//		1, 2, 6,
//		6, 5, 1,
//
//		// left
//		0, 4, 7,
//		7, 3, 0,
//
//		// top
//		4, 5, 6,
//		6, 7, 4,
//
//		// bottom
//		0, 3, 2,
//		2, 1, 0,
//
//		// back
//		0, 1, 5,
//		5, 4, 0,
//
//		// front
//		3, 7, 6,
//		6, 2, 3
//	};
//
//    m_indices_count = skybox_indices.size();
//
//	m_skybox_va.bind();
//
//    Buffer skybox_vertex_buff(skybox_verts.size() * sizeof(float), BufferType::VERTEX);
//	skybox_vertex_buff.set_data(0, skybox_verts);
//
//    Buffer skybox_index_buff(skybox_indices.size() * sizeof(unsigned int), BufferType::INDEX);
//    skybox_index_buff.set_data(0, skybox_indices);
//
//    skybox_vertex_buff.bind();
//    skybox_index_buff.bind();
//
//	BufferLayout sb_layout = { {0, 3, GL_FLOAT, false} };
//
//    m_skybox_va.set_layout(sb_layout);
//
//	m_skybox_va.unbind();
//	skybox_vertex_buff.unbind();
//    skybox_index_buff.unbind();
//}
//
//Skybox::Skybox(Skybox&& sb) noexcept
//	: m_skybox_texture(std::move(sb.m_skybox_texture))
//{
//	m_indices_count = sb.m_indices_count;
//	m_skybox_va = std::move(sb.m_skybox_va);
//    m_skybox_shader = sb.m_skybox_shader;
//	m_path = std::move(sb.m_path);
//    m_img_fmt = std::move(sb.m_img_fmt);
//}
//
//void Skybox::bind() const
//{
//    m_skybox_va.bind();
//    m_skybox_texture.bind(0);
//    m_skybox_shader->bind();
//}
//
//void Skybox::unbind() const
//{
//    m_skybox_va.unbind();
//    m_skybox_texture.unbind();
//    m_skybox_shader->unbind();
//}
//
//
//Skybox& Skybox::operator=(Skybox&& sb) noexcept
//{
//	m_indices_count = sb.m_indices_count;
//	m_skybox_va = std::move(sb.m_skybox_va);
//	m_skybox_texture = std::move(sb.m_skybox_texture);
//	m_skybox_shader = sb.m_skybox_shader;
//    m_img_fmt = sb.m_img_fmt;
//
//    return *this;
//}
//
//
