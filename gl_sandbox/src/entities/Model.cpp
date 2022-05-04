#include "pch.h"
#include "Model.h"

#include "GLError.h"
#include "GLTFLoader.h"

#include "components/Mesh.h"

#include "mathz/Misc.h"

#include <glad/glad.h>

void Model::draw() const
{
	if (has<Mesh>())
	{
		Mesh& m = get<Mesh>();

		m.draw();
	}
}

