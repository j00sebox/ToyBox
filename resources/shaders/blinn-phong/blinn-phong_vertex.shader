#version 420

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coord;

layout (std140, binding=0) uniform Transforms
{
// alignment offset
    mat4 u_view;		// 0
    mat4 u_projection;	// 64
};
uniform mat4 u_model;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_tex_coord;
out mat3 v_model;

void main()
{
    v_position = vec3(u_model * vec4(a_position, 1));
    v_normal = mat3(u_model) * a_normal;
    v_tex_coord = a_tex_coord;
    v_model = mat3(u_model);
    gl_Position = u_projection * u_view * u_model * vec4(a_position, 1);
}