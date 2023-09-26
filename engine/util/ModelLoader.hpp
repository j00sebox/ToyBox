#pragma once
#include "CommonTypes.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Vertex.hpp"
#include "scene/SceneNode.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

class ModelLoader
{
public:
    explicit ModelLoader(Renderer* renderer, const char* file_path);
    void load(SceneNode* scene_node);
    SceneNode* load_node(aiNode* current_node);
    void load_mesh(u32 mesh_index, Mesh& mesh);
    void load_material(u32 material_index, Material& material);
    const char* get_name();

    //static void load_primitive(Renderer* renderer, PrimitiveTypes primitive, Mesh& mesh);
    static void load_texture(Renderer* renderer, const char* texture_path, Material& material);

private:
    static std::vector<Vertex> get_vertices(aiMesh* mesh);
    static std::vector<u32> get_indices(aiMesh* mesh);
    static inline glm::mat4 aimatrix4x4_to_glmmat4(aiMatrix4x4 assimp_matrix);

    Renderer* m_renderer;
    Assimp::Importer m_importer;
    const aiScene* m_scene;
    const char* m_file_path;
    std::string m_base_dir;
};
