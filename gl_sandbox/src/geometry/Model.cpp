#include "pch.h"
#include "Model.h"

#include "GLError.h"
#include "GLTFLoader.h"

#include <glad/glad.h>

Model::Model()
{
	
}

void Model::draw() const
{
	m_va->bind();
	m_ib->bind();
	m_meshes[0].get_texture()->bind(0);
	GL_CALL(glDrawElements(GL_TRIANGLES, m_ib->get_count(), GL_UNSIGNED_INT, nullptr));
}

void Model::translate(const mathz::Vec3& pos)
{
	mathz::Mat4 t;
	t(3, 0) = pos.x;	t(3, 1) = pos.y;	t(3, 2) = pos.z;

	m_model_transform *= t;
}

void Model::load_mesh(const std::string& file_path)
{
	Mesh mesh(file_path);

	m_va.reset(new VertexArray());

	std::vector<float> vertices = mesh.get_vertices();

	m_vb = VertexBuffer();
	m_vb.add_data(vertices.data(), vertices.size() * sizeof(float));

	std::vector<unsigned int> indices = mesh.get_indices();

	m_ib.reset(new IndexBuffer(indices.data(), indices.size() * sizeof(unsigned int)));

	BufferLayout layout = {
		{0, 3, GL_FLOAT, false},
		{1, 2, GL_FLOAT, false}
	};

	m_va->set_layout(m_vb, layout);

	m_meshes.push_back(mesh);
}


