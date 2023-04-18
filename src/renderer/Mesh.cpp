#include "pch.h"
#include "Mesh.h"
#include "Log.h"
#include "GLError.h"

#include <glad/glad.h>

Mesh::Mesh(Mesh&& mesh) noexcept
{
    m_va = std::move(mesh.m_va);
    m_indices_count = mesh.m_indices_count;
}

void Mesh::load(const std::vector<float>& verts, const std::vector<unsigned int>& indices)
{
    m_indices_count = indices.size();

    m_va.bind();

    Buffer vertex_buffer{verts.size() * sizeof(float), BufferType::VERTEX};
    vertex_buffer.set_data(0, verts);

    Buffer index_buffer{indices.size() * sizeof(unsigned int), BufferType::INDEX};
    index_buffer.set_data(0, indices);

    vertex_buffer.bind();
    index_buffer.bind();

    BufferLayout layout = {
            {0, 3, GL_FLOAT, false},
            {1, 3, GL_FLOAT, false},
            {2, 2, GL_FLOAT, false}
    };

    m_va.set_layout(layout);

    m_va.unbind();
    vertex_buffer.unbind();
    index_buffer.unbind();
}

void Mesh::load_primitive(PrimitiveTypes primitive)
{
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

void Mesh::make_instanced(int instances, const std::vector<glm::mat4>& instance_matrices)
{
    m_instance_buffer.reset(new Buffer(instances * sizeof(glm::mat4), BufferType::VERTEX));
    m_instance_buffer->set_data(0, instance_matrices);
    m_instance_buffer->bind();

    m_va.bind();

    std::size_t vec4_size = sizeof(glm::vec4);
    GL_CALL(glEnableVertexAttribArray(3));
    GL_CALL(glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)0));
    GL_CALL(glEnableVertexAttribArray(4));
    GL_CALL(glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(1 * vec4_size)));
    GL_CALL(glEnableVertexAttribArray(5));
    GL_CALL(glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(2 * vec4_size)));
    GL_CALL(glEnableVertexAttribArray(6));
    GL_CALL(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4_size, (void*)(3 * vec4_size)));

    GL_CALL(glVertexAttribDivisor(3, 1));
    GL_CALL(glVertexAttribDivisor(4, 1));
    GL_CALL(glVertexAttribDivisor(5, 1));
    GL_CALL(glVertexAttribDivisor(6, 1));

    m_va.unbind();
    m_instance_buffer->unbind();

    m_instanced = true;
}

void Mesh::update_instances(const std::vector<glm::mat4>& instance_matrices)
{
    m_instance_buffer->set_data(0, instance_matrices);
}

void Mesh::bind() const
{
    m_va.bind();
}

void Mesh::unbind() const
{
    m_va.unbind();
}

std::unordered_map<std::string, std::shared_ptr<Mesh>> MeshTable::m_meshes;

void MeshTable::add(const std::string& name, Mesh&& m)
{
    if (!exists(name))
        m_meshes[name] = std::make_shared<Mesh>(std::move(m));

}

std::shared_ptr<Mesh> MeshTable::get(const std::string& name)
{
    if (exists(name))
    {
        return m_meshes[name];
    }

    fatal("Shader {} does not exist in library!\n", name);
    return nullptr;
}

bool MeshTable::exists(const std::string &name)
{
    return (m_meshes.find(name) != m_meshes.end());
}

std::string MeshTable::find(const std::shared_ptr<Mesh>& m)
{
    for (const auto& [name, mesh_ptr] : m_meshes)
    {
        if (mesh_ptr == m)
        {
            return name;
        }
    }

    return "";
}

bool MeshTable::is_instance(const std::string &name)
{
    if (exists(name))
    {
        return m_meshes[name]->is_instanced();
    }

    fatal("Shader {} does not exist in library!\n", name);
    return false;
}

void MeshTable::release()
{
    m_meshes.clear();
}




