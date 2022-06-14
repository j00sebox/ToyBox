#version 420

layout(location = 0) in vec3 a_position;

uniform mat4 u_model;
uniform mat4 u_light_space_view;
uniform mat4 u_light_space_projection;

void main()
{
    gl_Position = u_light_space_projection * u_light_space_view * u_model * vec4(a_position, 1.0);
}