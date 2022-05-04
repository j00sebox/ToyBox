#pragma once

#include "Shader.h"
#include "entities/Entity.h"

#include "mathz/Matrix.h"
#include "mathz/Quaternion.h"

class Model : public Entity
{
public:
	void draw() const override;

};