#version 420

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coord;
layout(location = 3) in mat4 instanceMatrix;

layout (std140, binding=0) uniform Transforms
{
// alignment offset
    mat4 u_view;		// 0
    mat4 u_projection;	// 64
};
uniform mat4 u_model;
uniform mat4 u_light_proj;

out vec3 v_position;
out vec3 v_normal;
out vec2 v_tex_coord;
out vec4 v_light_space_pos;

void main()
{
    v_position = vec3(instanceMatrix * vec4(a_position, 1));
    v_normal = transpose(inverse(mat3(instanceMatrix))) * a_normal;
    v_tex_coord = a_tex_coord;
    v_light_space_pos = u_light_proj * vec4(v_position, 1);
    gl_Position = u_projection * u_view * instanceMatrix * vec4(a_position, 1);
}