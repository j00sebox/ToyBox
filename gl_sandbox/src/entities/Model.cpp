#include "pch.h"
#include "Model.h"

#include "GLError.h"
#include "GLTFLoader.h"

#include "components/Mesh.h"

#include "mathz/Misc.h"

#include <glad/glad.h>

void Model::draw() const
{
	if (has<Mesh>())
	{
		auto& mesh = get<Mesh>();
		mesh.draw();
	}
}

void Model::load_gltf(const std::string& file_path)
{
}

