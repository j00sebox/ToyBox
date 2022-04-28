#pragma once

#include "Model.h"

class Skybox : public Model
{
public:
	Skybox();

	virtual void draw() const override;

private:
	unsigned int m_indices_count;
	VertexArray m_skybox_va;
	
};

