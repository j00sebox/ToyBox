#pragma once

#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Texture.h"

#include "Camera.h"

#include "math/Quaternion.h"

struct RendererUpdates
{
	float dx, dy;
	math::Vec3 pos;
};

class Renderer
{
public:
	Renderer(int width, int height);
	void update(float elpased_time);

	void move_cam_forward(float f);
	void move_cam_right(float r);
	void update_camera_rot(float dx, float dy);
	void update_camera_pos(const math::Vec3& pos);
	void update_camera_pos(float x, float y, float z);

	void reset_view();
	
private:
	void draw();

	int m_screen_width, m_screen_height;

	std::unique_ptr<Camera> m_camera = std::make_unique<Camera>();
	math::Quaternion m_camera_rot;

	std::shared_ptr<VertexArray> m_va;
	std::shared_ptr<ShaderProgram> m_shader;
	std::shared_ptr<IndexBuffer> m_ib;

	VertexBuffer m_vb;
	Texture2D m_lava_texure;
	math::Mat4 m_perspective;
	math::Mat4 m_object_transform;
	math::Mat4 m_square_move;
	math::Quaternion m_qrot;
};

