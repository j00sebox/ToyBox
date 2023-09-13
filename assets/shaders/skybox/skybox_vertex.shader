#version 420

layout(location = 0) in vec3 a_position;

layout (std140, binding=0) uniform Transforms
{
	// alignment offset
	mat4 u_view;		// 0
	mat4 u_projection;	// 64
};

out vec3 v_tex_coords;

void main()
{
	gl_Position = u_projection * mat4(mat3(u_view)) * vec4(a_position, 1);
	v_tex_coords = vec3(a_position.x, a_position.y, -a_position.z);
}