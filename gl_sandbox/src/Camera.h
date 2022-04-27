#pragma once

#include "mathz/Matrix.h"
#include "mathz/Quaternion.h"

class Camera
{
public:
	Camera(int width, int height);
	~Camera() {}

	mathz::Mat4 camera_look_at();
	mathz::Mat4 look_at_no_translate();

	void update(float elapsed_time);

	void rotate(mathz::Quaternion q);
	void move_forward(float f);
	void move_right(float r);

	void set_pos(mathz::Vec3&& pos);
	const mathz::Vec3& get_pos() const;

	inline const mathz::Vec3& get_forward() { return m_forward; }
	inline const mathz::Mat4& get_transform() { return m_transform; }

	void reset();

private:
	mathz::Vec3 m_position;
	mathz::Vec3 m_forward, m_up, m_right;
	mathz::Mat4 m_transform;

	int m_screen_width, m_screen_height;
	float m_speed = 0.1f;
	float m_sensitivity = 50.f;
	bool m_mouse_down = false;
};
