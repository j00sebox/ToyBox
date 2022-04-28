#pragma once

#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Texture.h"

#include "Camera.h"
#include "Model.h"
#include "Skybox.h"

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

	Skybox m_skybox;
	Model airplane;
	std::shared_ptr<ShaderProgram> m_cube_shader;
	Texture2D m_lava_texure;

	std::shared_ptr<ShaderProgram> m_skybox_shader;
	CubeMap m_skybox_texture;
	
	mathz::Mat4 m_perspective;
	mathz::Mat4 m_orthographic;
};

