#version 410

uniform vec4 u_light_colour;

out vec4 colour;

void main()
{
	colour = u_light_colour;
}