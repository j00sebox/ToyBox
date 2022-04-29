#include "pch.h"
#include "Renderer.h"

#include "GLError.h"
#include "events/EventList.h"

#include "mathz/Quaternion.h";

#include <glad/glad.h>

#define DEG_TO_RAD(x) (x / 180.f) * 3.1415f

Renderer::Renderer(int width, int height)
	: m_screen_width(width), m_screen_height(height)
{
	GL_CALL(glViewport(0, 0, width, height));

	GL_CALL(glEnable(GL_DEPTH_TEST));
	GL_CALL(glDepthFunc(GL_LEQUAL));
	GL_CALL(glFrontFace(GL_CCW));

	m_directional_light.normalize();

	m_scene.load("resources/scenes/flying_high.scene");
	m_camera = std::shared_ptr<Camera>(m_scene.get_camera());
	m_camera->resize(width, height);
}

void Renderer::update(float elapsed_time)
{
	m_scene.update(elapsed_time);

	draw();
}

void Renderer::draw()
{
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	m_scene.draw();
}

void Renderer::reset_view()
{
	m_camera->reset();
}


