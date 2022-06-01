#version 420

layout(location = 0) in vec3 a_position;

layout (std140, binding=0) uniform Transforms
{
	// alignment offset
	mat4 u_view;		// 0
	mat4 u_projection;	// 64
};
uniform mat4 u_model;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);
}