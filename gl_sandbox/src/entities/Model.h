#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "entities/Entity.h"

#include "mathz/Matrix.h"
#include "mathz/Quaternion.h"

class Model : public Entity
{
public:
	void draw() const override;
	void load_mesh(const std::string& file_path);

private:
	std::vector<Mesh> m_meshes;
};