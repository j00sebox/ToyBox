#version 420

layout(location = 0) in vec3 a_position;
layout(location = 3) in mat4 instanceMatrix;

uniform mat4 u_light_space_view;
uniform mat4 u_light_space_projection;

void main()
{
    gl_Position = u_light_space_projection * u_light_space_view * instanceMatrix * vec4(a_position, 1.0);
}