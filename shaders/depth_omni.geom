#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

layout (location = 1) uniform mat4 uLightMatrices[6];

out GEOM_OUT
{
    vec4 vFragPos;
}
geom_out;

void main()
{
    for (int face = 0; face < 6; ++face)
    {
        gl_Layer = face;
        for (int i = 0; i < 3; ++i)
        {
            geom_out.vFragPos = gl_in[i].gl_Position;
            gl_Position = uLightMatrices[face] * geom_out.vFragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}
