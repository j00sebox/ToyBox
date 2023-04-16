#version 430 core

in vec4 frag_pos;

uniform vec3 u_light_pos;
uniform float u_far_plane;

void main()
{
    // get distance between fragment and light source
    float light_distance = length(frag_pos.xyz - u_light_pos);

    // map to [0;1] range by dividing by far_plane
    light_distance = light_distance / u_far_plane;

    // write this as modified depth
    gl_FragDepth = light_distance;
}