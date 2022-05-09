#include "pch.h"
#include "Model.h"

#include "GLError.h"

#include "components/Mesh.h"
#include "components/Material.h"

#include "mathz/Misc.h"

#include <glad/glad.h>

GLTFLoader Model::load_gltf(const std::string& file_path)
{
	GLTFLoader loader(file_path.c_str());

	return loader;
}