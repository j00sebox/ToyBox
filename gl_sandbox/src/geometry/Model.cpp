#include "pch.h"
#include "Model.h"

#include "GLError.h"
#include "GLTFLoader.h"

#include <glad/glad.h>

void Model::draw() const
{
	m_shader_program->bind();
	for (const Mesh& m : m_meshes)
	{
		m.draw();
	}
}

void Model::translate(const mathz::Vec3& pos)
{
	m_translate[3][0] = pos.x;	m_translate[3][1] = pos.y;	m_translate[3][2] = pos.z;
}

void Model::scale(float s)
{
	m_scale[0][0] = s; m_scale[1][1] = s; m_scale[2][2] = s;
}

void Model::rotate(const mathz::Quaternion& q)
{
	m_rotation = q.convert_to_mat();
}

void Model::load_mesh(const std::string& file_path)
{
	m_meshes.emplace_back(Mesh(file_path));
}

void Model::attach_shader_program(ShaderProgram&& sp)
{
	m_shader_program = std::make_shared<ShaderProgram>(std::move(sp));
}

mathz::Mat4 Model::get_transform() const
{
	return m_translate * m_rotation * m_scale;
}


