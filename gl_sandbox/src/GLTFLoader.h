#pragma once

#include <json/json.hpp>

using json = nlohmann::json;

class GLTFLoader
{
public:
	GLTFLoader(const char* path);

	std::vector<float> get_positions();
	std::vector<float> get_tex_coords();
	std::vector<unsigned int> get_indices();
	void extract_floats(const json& accessor, std::vector<float>& flts);

private:
	void load_bin(const char* file_path);

	json m_json;
	std::string m_uri;

	std::vector<unsigned char> m_data;

	unsigned int m_postion_ind;
	unsigned int m_tex_coord_ind;
	unsigned int m_indices_ind;
};