#include "ModelLoader.h"

ModelLoader::ModelLoader(const char* file_path) :
    m_scene(m_importer.ReadFile(file_path,
                               aiProcess_CalcTangentSpace       |
                               aiProcess_Triangulate            |
                               aiProcess_JoinIdenticalVertices  |
                               aiProcess_SortByPType)),
   m_primitive_type(PrimitiveTypes::None)
{
    std::string p(file_path);
    m_base_dir = p.substr(0, (p.find_last_of('/') + 1));
}

ModelLoader::ModelLoader(PrimitiveTypes primitive_type) :
    m_scene(nullptr),
    m_primitive_type(primitive_type)
{
}

void ModelLoader::load_mesh(Mesh& mesh)
{
    if(m_scene)
    {
        std::vector<float> vertices = get_vertices(m_scene->mMeshes[0]);
        std::vector<unsigned> indices = get_indices(m_scene->mMeshes[0]);

        mesh.load(vertices, indices);
    }
    else
    {
        switch (m_primitive_type)
        {
            case PrimitiveTypes::None:
                break;
            case PrimitiveTypes::Cube:
            {
                mesh.load(Cube::vertices, Cube::indices);
                break;
            }
            case PrimitiveTypes::Quad:
            {
                mesh.load(Quad::vertices, Quad::indices);
                break;
            }
        }
    }
}

void ModelLoader::load_material(Material& material)
{
    aiString diffuse_texture_path, specular_texture_path, normal_texture_path, occlusion_texture_path;
    m_scene->mMaterials[0]->GetTexture(aiTextureType_DIFFUSE, 0, &diffuse_texture_path);
    m_scene->mMaterials[0]->GetTexture(aiTextureType_SPECULAR, 0, &specular_texture_path);
    m_scene->mMaterials[0]->GetTexture(aiTextureType_NORMALS, 0, &normal_texture_path);
    m_scene->mMaterials[0]->GetTexture(aiTextureType_AMBIENT, 0, &occlusion_texture_path);

    std::string textures[4];

    textures[0] = m_base_dir + diffuse_texture_path.C_Str();
    textures[1] = (specular_texture_path.length > 0) ? m_base_dir + specular_texture_path.C_Str() : "none";
    textures[2] = (normal_texture_path.length > 0) ? m_base_dir + normal_texture_path.C_Str() : "none";
    textures[3] = (occlusion_texture_path.length > 0) ? m_base_dir + occlusion_texture_path.C_Str() : "none";

    material.load(textures);
}

std::vector<float> ModelLoader::get_vertices(aiMesh* mesh)
{
    std::vector<float> vertices;
    for(int i = 0; i < mesh->mNumVertices; ++i)
    {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);
        vertices.push_back(mesh->mNormals[i].x);
        vertices.push_back(mesh->mNormals[i].y);
        vertices.push_back(mesh->mNormals[i].z);
        vertices.push_back(mesh->mTextureCoords[0][i].x);
        vertices.push_back(mesh->mTextureCoords[0][i].y);
    }

    return vertices;
}

std::vector<unsigned> ModelLoader::get_indices(aiMesh* mesh)
{
    std::vector<unsigned> indices;
    for(int i = 0; i < mesh->mNumFaces; ++i)
    {
        for(int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
        {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }

    return indices;
}

const char* ModelLoader::get_name()
{
    return m_scene->mMeshes[0]->mName.C_Str();
}