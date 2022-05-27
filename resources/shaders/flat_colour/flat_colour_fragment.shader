#version 410

out vec4 colour;

uniform vec4 u_flat_colour;

void main()
{
	colour = u_flat_colour;
}