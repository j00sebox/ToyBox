#include "pch.h"
#include "Model.h"

#include "GLError.h"
#include "GLTFLoader.h"

#include <glad/glad.h>

void Model::draw() const
{
	for (const Mesh& m : m_meshes)
	{
		m.draw();
	}
}

void Model::translate(const mathz::Vec3& pos)
{
	mathz::Mat4 t;
	t(3, 0) = pos.x;	t(3, 1) = pos.y;	t(3, 2) = pos.z;

	m_model_transform *= t;
}

void Model::load_mesh(const std::string& file_path)
{
	m_meshes.push_back(Mesh(file_path));
}


