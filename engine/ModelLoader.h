#pragma once

#include "Mesh.h"
#include "Material.h"

#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

class ModelLoader
{
public:
    explicit ModelLoader(const char* file_path);
    explicit ModelLoader(PrimitiveTypes primitive_type);
    void load_mesh(Mesh& mesh);
    void load_material(Material& material);
    const char* get_name();

private:
    static std::vector<float> get_vertices(aiMesh* mesh);
    static std::vector<unsigned> get_indices(aiMesh* mesh);

    Assimp::Importer m_importer;
    const aiScene* m_scene;
    std::string m_base_dir;
    PrimitiveTypes m_primitive_type;
};

