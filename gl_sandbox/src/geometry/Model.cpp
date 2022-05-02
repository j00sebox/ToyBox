#include "pch.h"
#include "Model.h"

#include "GLError.h"
#include "GLTFLoader.h"

#include "mathz/Misc.h"

#include <glad/glad.h>

void Model::draw() const
{
	for (const Mesh& m : m_meshes)
	{
		m.draw();
	}
}

void Model::load_mesh(const std::string& file_path)
{
	m_meshes.emplace_back(Mesh(file_path));
}


