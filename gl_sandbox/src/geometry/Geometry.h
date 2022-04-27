#pragma once

#include "VertexArray.h"
#include "Buffer.h"
#include "Texture.h"

#include "math/Matrix.h"

#include <memory>
#include <vector>

struct Vertex
{
	math::Vec3 positon;
	math::Vec2<float> st;
};

class Geometry
{
public:
	virtual void draw() const = 0;
	virtual void translate(const math::Vec3& pos) = 0;

};

class Object : public Geometry
{
public:
	Object(const std::string& file_path);

	virtual void draw() const override;
	virtual void translate(const math::Vec3& pos) override;

	math::Mat4 get_transform() const { return m_model_transform; }

protected:
	std::vector<math::Vec3> floats_to_vec3(const std::vector<float>& flts);
	std::vector<math::Vec2<float>> floats_to_vec2(const std::vector<float>& flts);

	std::unique_ptr<VertexArray> m_va;
	std::unique_ptr<IndexBuffer> m_ib;
	std::unique_ptr<Texture2D> m_texture;
	VertexBuffer m_vb;
	

	math::Mat4 m_model_transform;
};