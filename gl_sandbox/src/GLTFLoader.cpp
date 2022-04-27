#include "pch.h"
#include "GLTFLoader.h"

#include "Log.h"

std::string file_to_string(const char* file_path)
{
	std::string line;
	std::ifstream stream(file_path);

	std::stringstream ss;

	while (getline(stream, line))
	{
		ss << line << "\n";
	}

	stream.close();

	return ss.str();
}

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
	
	ASSERT(false);
}

GLTFLoader::GLTFLoader(const char* path)
{
	std::string src = file_to_string(path);

	m_json = json::parse(src);

	std::string uri = m_json["buffers"][0]["uri"];

	std::string p(path);
	m_base_dir = p.substr(0, (p.find_last_of('/') + 1));
	std::string bin_path = m_base_dir + uri;

	load_bin(bin_path.c_str());

	

	json attributes = m_json["meshes"][0]["primitives"][0]["attributes"];

	m_postion_ind = attributes["POSITION"];
	m_tex_coord_ind = attributes["TEXCOORD_0"];
	m_indices_ind = m_json["meshes"][0]["primitives"][0]["indices"];

	json materials = m_json["materials"][0];

	m_bc_tex_ind = materials["pbrMetallicRoughness"]["baseColorTexture"]["index"];
	/*m_norm_tex_ind = materials["normalTexture"]["index"];
	m_occ_tex_ind = materials["occlusionTexture"]["index"];*/
}

std::vector<float> GLTFLoader::get_positions()
{
	json accessor = m_json["accessors"][m_postion_ind];
	
	std::vector<float> positions;
	extract_floats(accessor, positions);

	return positions;
}

std::vector<float> GLTFLoader::get_tex_coords()
{
	json accessor = m_json["accessors"][m_tex_coord_ind];

	std::vector<float> tex_coords;
	extract_floats(accessor, tex_coords);

	return tex_coords;
}

std::vector<unsigned int> GLTFLoader::get_indices()
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

		unsigned int length = count * 4;

		for (unsigned int i = buffer_byte_offset; i < (buffer_byte_offset + length);)
		{
			unsigned char byte[] = { m_data[i++], m_data[i++], m_data[i++], m_data[i++] };
			unsigned int val;

			std::memcpy(&val, byte, sizeof(unsigned int));

			indices.push_back(val);
		}
		break;
		/*case 5123:
			for (unsigned int i = buffer_byte_offset; i < (buffer_byte_offset + length);)
			{
				unsigned char byte[] = { m_data[i++], m_data[i++], m_data[i++], m_data[i++] };
				float val;

				std::memcpy(&val, byte, sizeof(unsigned short));

				indices.push_back(val);
			}
			break;
		case 5122:
			for (unsigned int i = buffer_byte_offset; i < (buffer_byte_offset + length);)
			{
				unsigned char byte[] = { m_data[i++], m_data[i++], m_data[i++], m_data[i++] };
				float val;

				std::memcpy(&val, byte, sizeof(short));

				indices.push_back(val);
			}
			break;*/
		}

	return indices;
}

std::string GLTFLoader::get_base_color_texture()
{
	json uri = m_json["images"][m_bc_tex_ind];
	std::string image = uri["uri"];

	return m_base_dir + image;
}

std::string GLTFLoader::get_normal_texture()
{
	return std::string();
}

std::string GLTFLoader::get_occlusion_texture()
{
	return std::string();
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

void GLTFLoader::extract_floats(const json& accessor, std::vector<float>& flts)
{
	unsigned int index = accessor.value("bufferView", 0);
	unsigned int byte_offset = accessor.value("byteOffset", 0);
	unsigned int count = accessor["count"];
	std::string type = accessor["type"];

	json buffer = m_json["bufferViews"][index];
	unsigned int buffer_byte_offset = buffer["byteOffset"] + byte_offset;
	unsigned int length = count * get_num_verts(type) * 4;

	for (unsigned int i = buffer_byte_offset; i < (buffer_byte_offset + length);)
	{
		unsigned char byte[] = { m_data[i++], m_data[i++], m_data[i++], m_data[i++] };
		float val;

		std::memcpy(&val, byte, sizeof(float));

		flts.push_back(val);
	}
}
