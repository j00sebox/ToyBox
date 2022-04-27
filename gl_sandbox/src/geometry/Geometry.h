#pragma once

#include "VertexArray.h"
#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"

#include "math/Matrix.h"

#include <memory>
#include <vector>

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

protected:
	std::unique_ptr<VertexArray> m_va;
	VertexBuffer m_vb;
	std::unique_ptr<IndexBuffer> m_ib;

	math::Mat4 m_model_transform;
};