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
    ~Mesh() { m_instance_buffer.reset(); }

    void load(const std::vector<float>& verts, const std::vector<unsigned int>& indices);
    void load_primitive(PrimitiveTypes primitive);
    void make_instanced(int instances, const std::vector<glm::mat4> instance_matrices);
    void update_instances(const std::vector<glm::mat4>& instance_matrices);

    void bind() const;
    void unbind() const;

    [[nodiscard]] unsigned int get_index_count() const { return m_indices_count; }
    [[nodiscard]] bool is_instanced() const { return m_instanced; }

private:
    unsigned int m_indices_count = 0;
    bool m_instanced = false;
    VertexArray m_va;
    std::unique_ptr<Buffer> m_instance_buffer;
};

class MeshTable
{
public:
    static void add(const std::string& name, Mesh&& m);
    static std::shared_ptr<Mesh> get(const std::string& name);
    static bool exists(const std::string& name);
    static std::string find(const std::shared_ptr<Mesh>& s);
    static bool is_instance(const std::string& name);
    static void release();

private:
    static std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;
};