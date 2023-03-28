#pragma once

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>

class Camera
{
public:
	Camera();
	~Camera() = default;

	glm::mat4 camera_look_at();

    // true if the camera could be moved
	bool update(float elapsed_time);

	void rotate(glm::quat q);
	void move_forward(float f);
	void move_right(float r);

	void set_pos(glm::vec3&& pos);
	[[nodiscard]] const glm::vec3& get_pos() const;

	inline const glm::vec3& get_forward() { return m_forward; }
	inline const glm::mat4& get_transform() { return m_transform; }
	inline const glm::mat4& get_perspective() { return m_perspective; }
	inline const glm::mat4& get_orthographic() { return m_orthographic; }

	void resize(int width, int height);
	void reset();

private:
    glm::vec3 m_position;
    glm::vec3 m_forward, m_up, m_right;
    glm::mat4 m_transform;

    glm::mat4 m_perspective;
    glm::mat4 m_orthographic;

	int m_screen_width, m_screen_height;
	float m_near = 0.1f, m_far = 1000.f;
	float m_fov = 90.f;
	float m_speed = 0.01f;
	float m_sensitivity = 30.f;
	bool m_mouse_down = false;
};
