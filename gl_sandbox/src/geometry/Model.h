#pragma once

#include "Mesh.h"

#include "mathz/Matrix.h"

class Model
{
public:
	virtual void draw() const;
	void translate(const mathz::Vec3& pos);

	void load_mesh(const std::string& file_path);

	mathz::Mat4 get_transform() const { return m_model_transform; }

private:
	std::vector<Mesh> m_meshes;

	mathz::Mat4 m_model_transform;
};