#pragma once

#include "Mesh.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Texture.h"

#include "mathz/Matrix.h"

class Model
{
public:
	Model();

	virtual void draw() const;
	void translate(const mathz::Vec3& pos);

	void load_mesh(const std::string& file_path);

	mathz::Mat4 get_transform() const { return m_model_transform; }

private:
	std::vector<Mesh> m_meshes;

	std::unique_ptr<VertexArray> m_va;
	std::unique_ptr<IndexBuffer> m_ib;
	VertexBuffer m_vb;
	
	mathz::Mat4 m_model_transform;
};