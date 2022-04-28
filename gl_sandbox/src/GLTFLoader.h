#pragma once

#include <json/json.hpp>

using json = nlohmann::json;

class GLTFLoader
{
public:
	GLTFLoader(const char* path);

	std::vector<float> get_positions();
	std::vector<float> get_normals();
	std::vector<float> get_tex_coords();
	std::vector<unsigned int> get_indices();

	std::string get_base_color_texture();
	std::string get_normal_texture();
	std::string get_occlusion_texture();

private:
	void load_bin(const char* file_path);
	void extract_floats(const json& accessor, std::vector<float>& flts);

	json m_json;
	std::string m_base_dir;

	std::vector<unsigned char> m_data;

	unsigned int m_postion_ind;
	unsigned int m_tex_coord_ind;
	unsigned int m_indices_ind;

	unsigned int m_bc_tex_ind;
	unsigned int m_norm_tex_ind;
	unsigned int m_occ_tex_ind;
};