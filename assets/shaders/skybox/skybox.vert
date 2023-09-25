#version 450

layout(location = 0) in vec3 in_position;

layout(location = 0) out vec3 v_tex_coords;

layout(set=0, binding=0) uniform CameraDataBuffer
{
    mat4 view;
    mat4 proj;
    vec3 camera_position;
} camera_data;

void main()
{
    gl_Position = camera_data.proj * mat4(mat3(camera_data.view)) * vec4(in_position, 1);
    v_tex_coords = vec3(in_position.x, in_position.y, -in_position.z);
}