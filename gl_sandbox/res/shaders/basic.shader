#shader vertex

#version 410

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_colour;

out vec4 v_colour;

void main()
{
	v_colour = a_colour;
	gl_Position = vec4(a_position, 1);
}

#shader fragment

#version 410

layout(location = 0) in vec4 v_colour;

out vec4 colour;

void main()
{
	colour = v_colour;
}