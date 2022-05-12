#pragma once

#include "Entity.h"
#include "Shader.h"

#include "mathz/Matrix.h"
#include "mathz/Quaternion.h"

class GLTFLoader;

class Model : public Entity
{
public:
	GLTFLoader load_gltf(const std::string& file_path);
};