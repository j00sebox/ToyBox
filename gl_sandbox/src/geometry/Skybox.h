#pragma once

#include "Model.h"

class Skybox : public Model
{
public:
	Skybox();

	virtual void draw() const override;

private:
	std::shared_ptr<VertexArray> m_skybox_va;
	std::shared_ptr<IndexBuffer> m_skybox_ib;
	VertexBuffer m_skybox_vb;
};

