#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "SceneObject.h"

#include "mathz/Matrix.h"
#include "mathz/Quaternion.h"

class Model : public SceneObject
{
public:
	void draw() const override;
	void load_mesh(const std::string& file_path);

private:
	std::vector<Mesh> m_meshes;
};