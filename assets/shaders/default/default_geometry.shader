#version 430

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 normal;
} vs_in[];

const float MAGNITUDE = 0.4;

void func(int index)
{
    gl_Position =  gl_in[index].gl_Position + vec4(vs_in[index].normal, 0.0) * MAGNITUDE;
    EmitVertex();

}

void main()
{
    func(0);
    func(1);
    func(2);
    EndPrimitive();
}