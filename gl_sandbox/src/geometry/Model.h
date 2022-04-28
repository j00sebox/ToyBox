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
	void attach_shader(std::shared_ptr<ShaderProgram> shader);

	mathz::Mat4 get_transform() const;

private:
	std::vector<Mesh> m_meshes;
	std::shared_ptr<ShaderProgram> m_shader;

	mathz::Mat4 m_scale;
	mathz::Mat4 m_rotation;
	mathz::Mat4 m_translate;
};