#version 410

layout(location = 0) in vec3 a_position;

uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_tex_coords;

void main()
{
	vec4 pos = u_projection * u_view * vec4(a_position, 1);
	gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
	v_tex_coords = vec3(a_position.x, a_position.y, -a_position.z);
}