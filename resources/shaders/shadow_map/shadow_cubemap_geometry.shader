#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadow_transforms[6];

out vec4 frag_pos;

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for(int v_idx = 0; v_idx < 3; ++v_idx)
        {
            frag_pos = gl_in[v_idx].gl_Position;
            gl_Position = shadow_transforms[face] * frag_pos;
            EmitVertex();
        }
        EndPrimitive();
    }
}