#pragma once

#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Texture.h"

#include "Camera.h"
#include "Model.h"
#include "Skybox.h"
#include "Scene.h"

#include "mathz/Quaternion.h"

class Renderer
{
public:
	Renderer(int width, int height);
	
	void update(float elpased_time);
	void reset_view();
	
private:
	void draw();

	int m_screen_width, m_screen_height;

	float m_near = 0.1f, m_far = 1000.f;

	std::unique_ptr<Camera> m_camera;

	Scene m_scene;

	Skybox m_skybox;
	std::shared_ptr<ShaderProgram> m_skybox_shader;

	Model m_airplane, m_scroll;
	std::shared_ptr<ShaderProgram> m_airplane_shader, m_scroll_shader;

	mathz::Mat4 m_perspective;
	mathz::Mat4 m_orthographic;

	mathz::Vec3 m_directional_light = { 0.f, 10.f, -1.f };
};

