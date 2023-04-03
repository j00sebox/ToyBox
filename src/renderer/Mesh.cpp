#include "pch.h"
#include "Mesh.h"

#include <glad/glad.h>

Mesh::Mesh(Mesh&& mesh) noexcept
{
    m_va = std::move(mesh.m_va);
    m_vb = std::move(mesh.m_vb);
    m_ib = std::move(mesh.m_ib);
    m_indices_count = mesh.m_indices_count;
}

void Mesh::load(const std::vector<float>& verts, const std::vector<unsigned int>& indices)
{
    m_indices_count = indices.size();

    m_va.bind();

    m_vb.set_data(verts);
    m_ib.set_data(indices);

    BufferLayout layout = {
            {0, 3, GL_FLOAT, false},
            {1, 3, GL_FLOAT, false},
            {2, 2, GL_FLOAT, false}
    };

    m_va.set_layout(m_vb, layout);

    m_va.unbind();
    m_vb.unbind();
    m_ib.unbind();
}

void Mesh::load_primitive(PrimitiveTypes primitive)
{
    // TODO: Figure out better way to do this
    //m_primitive = primitive;

    switch (primitive)
    {
        case PrimitiveTypes::None:
            break;
        case PrimitiveTypes::Cube:
        {
            load(Cube::vertices, Cube::indices);
            break;
        }
        case PrimitiveTypes::Quad:
        {
            load(Quad::vertices, Quad::indices);
            break;
        }
    }
}

void Mesh::bind() const
{
    m_va.bind();
}

void Mesh::unbind() const
{
    m_va.unbind();
}