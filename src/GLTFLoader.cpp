#include "pch.h"
#include "GLTFLoader.h"
#include "FileOperations.h"
#include "Log.h"

int get_num_verts(const std::string& type)
{
	if (type == "SCALAR")
		return 1;
	else if (type == "VEC2")
		return 2;
	else if (type == "VEC3")
		return 3;
	else if (type == "VEC4")
		return 4;
	
	return 0;
}

GLTFLoader::GLTFLoader(const char* path)
{
    read_file(path);
}

void GLTFLoader::read_file(const char* path)
{
    std::string src = file_to_string(path);

    m_json = json::parse(src);

    std::string uri = m_json["buffers"][0]["uri"];

    std::string p(path);
    m_base_dir = p.substr(0, (p.find_last_of('/') + 1));
    std::string bin_path = m_base_dir + uri;

    load_bin(bin_path.c_str());

    json attributes = m_json["meshes"][0]["primitives"][0]["attributes"];

    m_position_ind = attributes["POSITION"];
    m_tex_coord_ind = attributes["TEXCOORD_0"];
    m_normal_ind = attributes["NORMAL"];
    m_indices_ind = m_json["meshes"][0]["primitives"][0]["indices"];

    json materials = m_json["materials"][0];

    m_bc_tex_ind = materials["pbrMetallicRoughness"]["baseColorTexture"]["index"];

    if (!materials["pbrMetallicRoughness"]["metallicRoughnessTexture"].is_null())
    {
        m_spec_tex_ind = materials["pbrMetallicRoughness"]["metallicRoughnessTexture"]["index"];
    }

    if (!materials["normalTexture"].is_null())
    {
        m_norm_tex_ind = materials["normalTexture"]["index"];
    }

    if (!materials["occlusionTexture"].is_null())
    {
        m_occ_tex_ind = materials["occlusionTexture"]["index"];
    }
}

std::vector<float> GLTFLoader::get_positions() const
{
	json accessor = m_json["accessors"][m_position_ind];
	
	std::vector<float> positions;
	extract_floats(accessor, positions);

	return positions;
}

std::vector<float> GLTFLoader::get_normals() const
{
	json accessor = m_json["accessors"][m_normal_ind];

	std::vector<float> normals;
	extract_floats(accessor, normals);

	return normals;
}

std::vector<float> GLTFLoader::get_tex_coords() const
{
	json accessor = m_json["accessors"][m_tex_coord_ind];

	std::vector<float> tex_coords;
	extract_floats(accessor, tex_coords);

	return tex_coords;
}

std::vector<unsigned int> GLTFLoader::get_indices() const
{
	json accessor = m_json["accessors"][m_indices_ind];

	std::vector<unsigned int> indices;

	unsigned int index = accessor.value("bufferView", 0);
	unsigned int byte_offset = accessor.value("byteOffset", 0);
	unsigned int count = accessor["count"];
	unsigned int component_type = accessor["componentType"];
	std::string type = accessor["type"];

	json buffer = m_json["bufferViews"][index];
	unsigned int buffer_byte_offset = buffer.value("byteOffset", 0) + byte_offset;
	
	switch (component_type)
	{
	case 5125:
    {
        unsigned int length = count * 4;

        for (unsigned int i = buffer_byte_offset; i < (buffer_byte_offset + length);)
        {
            unsigned char byte[] = { m_data[i++], m_data[i++], m_data[i++], m_data[i++] };
            unsigned int val;

            std::memcpy(&val, byte, sizeof(unsigned int));

            indices.push_back(val);
        }
        break;
    }

    case 5123:
    {
        unsigned int length = count * 4;

        for (unsigned int i = buffer_byte_offset; i < (buffer_byte_offset + length);)
        {
            unsigned char byte[] = { m_data[i++], m_data[i++] };
            unsigned short val;

            std::memcpy(&val, byte, sizeof(unsigned short));

            indices.push_back(val);
        }
        break;
    }


    // TODO: add the other types
//		case 5122:
//			for (unsigned int i = buffer_byte_offset; i < (buffer_byte_offset + length);)
//			{
//				unsigned char byte[] = { m_data[i++], m_data[i++], m_data[i++], m_data[i++] };
//				float val;
//
//				std::memcpy(&val, byte, sizeof(short));
//
//				indices.push_back(val);
//			}
//			break;*/
    }

	return indices;
}

std::vector<std::string> GLTFLoader::get_textures() const
{
	std::vector<std::string> vec;

	vec.emplace_back(get_base_color_texture());

    if(m_spec_tex_ind != -1)
	    vec.emplace_back(get_specular_texture());

    if(m_norm_tex_ind != -1)
        vec.emplace_back(get_normal_texture());

    if(m_occ_tex_ind != -1)
        vec.emplace_back(get_occlusion_texture());
	
	return vec;
}

std::string GLTFLoader::get_base_color_texture() const
{
	json uri = m_json["images"][m_bc_tex_ind];
	std::string image = uri["uri"];

	return m_base_dir + image;
}

std::string GLTFLoader::get_specular_texture() const
{
    json uri;
    if(m_spec_tex_ind != -1)
        uri = m_json["images"][m_spec_tex_ind];
    else
        return "none";

	std::string image = uri["uri"];

	return m_base_dir + image;
}

std::string GLTFLoader::get_normal_texture() const
{
    json uri;
    if(m_spec_tex_ind != -1)
        uri = m_json["images"][m_norm_tex_ind];
    else
        return "none";

	std::string image = uri["uri"];

	return m_base_dir + image;
}

std::string GLTFLoader::get_occlusion_texture() const
{
	json uri;
    if(m_spec_tex_ind != -1)
        uri = m_json["images"][m_occ_tex_ind];
    else
        return "none";

	std::string image = uri["uri"];

	return m_base_dir + image;
}

void GLTFLoader::load_bin(const char* file_path)
{
	// open file
	std::streampos file_size;
	std::ifstream file(file_path, std::ios::binary);

	// get size
	file.seekg(0, std::ios::end);
	file_size = file.tellg();
	file.seekg(0, std::ios::beg);

	// read data
	m_data.assign(file_size, 0);
	file.read((char*)&m_data[0], file_size);
}

void GLTFLoader::extract_floats(const json& accessor, std::vector<float>& flts) const
{
	unsigned int index = accessor.value("bufferView", 0);
	unsigned int byte_offset = accessor.value("byteOffset", 0);
	unsigned int count = accessor["count"];
	std::string type = accessor["type"];

	json buffer = m_json["bufferViews"][index];
	unsigned int buffer_offset = buffer["byteOffset"];
	unsigned int buffer_byte_offset = buffer_offset + byte_offset;
	unsigned int length = count * get_num_verts(type) * 4;

	for (unsigned int i = buffer_byte_offset; i < (buffer_byte_offset + length);)
	{
		unsigned char byte[] = { m_data[i++], m_data[i++], m_data[i++], m_data[i++] };
		float val;

		std::memcpy(&val, byte, sizeof(float));

		flts.push_back(val);
	}
}


