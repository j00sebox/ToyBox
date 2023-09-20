#pragma once

#include "renderer/Fwd.h"

#include <vector>
#include <memory>
#include <glm/vec4.hpp>

//class Material
//{
//public:
//	void set_shader(const std::shared_ptr<ShaderProgram>& shader) { m_shader = shader; }
//	[[nodiscard]] const std::shared_ptr<ShaderProgram>& get_shader() const { return m_shader; }
//
//	void load(const std::string* textures);
//	void bind() const;
//	void unbind() const;
//
//	void set_colour(const glm::vec4& colour) { m_colour = colour; }
//	void set_metallic_property(float new_val) { m_metallic = new_val; }
//	void set_roughness(float new_val) { m_roughness = new_val; }
//	[[nodiscard]] const glm::vec4& get_colour() const { return m_colour; }
//
//private:
//	std::shared_ptr<ShaderProgram> m_shader;
//	bool m_using_textures = false;
//	std::unique_ptr<Texture2D> m_textures[4];
//    std::string m_texture_locations[4];
//
//    // custom properties
//    glm::vec4 m_colour = glm::vec4(1.f, 1.f, 1.f, 1.f);
//	float m_metallic = 0.f;
//	float m_roughness = 0.f;
//
//    friend class MaterialComponent;
//};
//
//class MaterialTable
//{
//public:
//    static void add(const std::string& name, Material&& m);
//    static std::shared_ptr<Material> get(const std::string& name);
//    static bool exists(const std::string& name);
//    static std::string find(const std::shared_ptr<Material>& s);
//    static void release();
//
//private:
//    static std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
//};