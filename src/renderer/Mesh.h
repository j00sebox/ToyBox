#pragma once

#include "Primitives.h"
#include "VertexArray.h"

#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 st;
};

class Mesh
{
public:
    Mesh() = default;
    Mesh(Mesh&& mesh) noexcept;

    void load(const std::vector<float>& verts, const std::vector<unsigned int>& indices);
    void load_primitive(PrimitiveTypes primitive);
    void bind() const;
    void unbind() const;

    [[nodiscard]] unsigned int get_vertex_count() const { return m_vertex_count; }
    [[nodiscard]] unsigned int get_index_count() const { return m_indices_count; }

private:
    unsigned int m_vertex_count = 0;
    unsigned int m_indices_count = 0;
    VertexArray m_va;
    VertexBuffer m_vb;
    IndexBuffer m_ib;
};