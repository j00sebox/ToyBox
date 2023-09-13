#version 420

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

layout (std140, binding=0) uniform Transforms
{
	// alignment offset
	mat4 u_view;		// 0
	mat4 u_projection;	// 64
};
uniform mat4 u_model;
uniform float u_outlining_factor;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(a_position + a_normal * u_outlining_factor, 1.0f);
}