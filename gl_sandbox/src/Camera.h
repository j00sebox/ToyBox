#pragma once

#include "math/Matrix.h"
#include "math/Quaternion.h"

class Camera
{
public:
	Camera();
	~Camera() {}

	math::Mat4 camera_look_at();

	void rotate(math::Quaternion q);
	void move_forward(float f);
	void move_right(float r);

	void set_pos(math::Vec3&& pos);
	const math::Vec3& get_pos() const;

	inline const math::Vec3& get_forward() { return m_forward; }
	inline const math::Mat4& get_transform() { return m_transform; }

	void reset();

private:
	math::Vec3 m_position;
	math::Vec3 m_forward, m_up, m_right;
	math::Mat4 m_transform;
};
