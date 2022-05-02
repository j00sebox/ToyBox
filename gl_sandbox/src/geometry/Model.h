#pragma once

#include "Mesh.h"
#include "Shader.h"

#include "mathz/Matrix.h"
#include "mathz/Quaternion.h"

class Model
{
public:
	virtual void draw() const;
	void translate(const mathz::Vec3& pos);
	void scale(float s);
	void rotate(const mathz::Quaternion& q);

	void load_mesh(const std::string& file_path);
	void set_shader(const std::string& name);
	const std::string& get_shader() const { return m_shader_name; }

	void set_name(const std::string& name);
	const std::string& get_name() const { return m_name; }

	const mathz::Vec3& get_position() const { return m_postion; }
	mathz::Mat4 get_transform() const;

private:
	std::string m_name;
	mathz::Vec3 m_postion;
	std::vector<Mesh> m_meshes;
	std::string m_shader_name;

	mathz::Mat4 m_scale;
	mathz::Mat4 m_rotation;
	mathz::Mat4 m_translate;
};