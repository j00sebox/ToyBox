#pragma once

#include "Geometry.h"

class Skybox : public Geometry
{
public:
	Skybox();

	virtual void draw() const override;
	virtual void translate(const mathz::Vec3& pos) override {}

private:
	std::shared_ptr<VertexArray> m_skybox_va;
	std::shared_ptr<IndexBuffer> m_skybox_ib;
	VertexBuffer m_skybox_vb;
};

