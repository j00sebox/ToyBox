#version 410

layout(location = 0) in vec4 v_colour;

out vec4 colour;

void main()
{
	colour = v_colour;
}