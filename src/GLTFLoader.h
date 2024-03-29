#pragma once

#include <nlohmann/json.hpp>

using namespace nlohmann;

class GLTFLoader
{
public:
    GLTFLoader() {}
	explicit GLTFLoader(const char* path);

    void read_file(const char* path);

	[[nodiscard]] std::vector<float> get_positions() const;
	[[nodiscard]] std::vector<float> get_normals() const;
	[[nodiscard]] std::vector<float> get_tex_coords() const;
	[[nodiscard]] std::vector<unsigned int> get_indices() const;

	[[nodiscard]] std::vector<std::string> get_textures() const;
	[[nodiscard]] std::string get_base_color_texture() const;
	[[nodiscard]] std::string get_specular_texture() const;
	[[nodiscard]] std::string get_normal_texture() const;
	[[nodiscard]] std::string get_occlusion_texture() const;

private:
	void load_bin(const char* file_path);
	void extract_floats(const json& accessor, std::vector<float>& flts) const;

	json m_json;
	std::string m_base_dir;

	std::vector<unsigned char> m_data;

	unsigned int m_position_ind;
	unsigned int m_normal_ind;
	unsigned int m_tex_coord_ind;
	unsigned int m_indices_ind;

	unsigned int m_bc_tex_ind;
	int m_spec_tex_ind = -1;
	int m_norm_tex_ind = -1;
	int m_occ_tex_ind = -1;
};