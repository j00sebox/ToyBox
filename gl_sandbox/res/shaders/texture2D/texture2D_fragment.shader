#version 410

uniform sampler2D tex;

in vec2 v_tex_coord;

out vec4 colour;

void main()
{
	colour = texture(tex, v_tex_coord);
}