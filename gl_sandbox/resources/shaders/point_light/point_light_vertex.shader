#version 410

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

layout(location = 0) in vec3 a_position;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1);
}