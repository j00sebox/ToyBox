#pragma once

#include "Mesh.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Texture.h"

#include "mathz/Matrix.h"

class Geometry
{
public:
	virtual void draw() const = 0;
	virtual void translate(const mathz::Vec3& pos) = 0;
};

class Object : public Geometry
{
public:
	Object(const std::string& file_path);

	virtual void draw() const override;
	virtual void translate(const mathz::Vec3& pos) override;

	mathz::Mat4 get_transform() const { return m_model_transform; }

private:
	void load_mesh(const std::string& file_path);

	std::vector<Mesh> m_meshes;

	std::unique_ptr<VertexArray> m_va;
	std::unique_ptr<IndexBuffer> m_ib;
	VertexBuffer m_vb;
	
	mathz::Mat4 m_model_transform;
};