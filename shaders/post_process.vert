#version 460 core

// assume CCW
const vec2 positions[6] = vec2[](
    vec2(-1.0f, -1.0f),
    vec2(1.0f, -1.0f),
    vec2(1.0f, 1.0f),

    vec2(1.0f, 1.0f),
    vec2(-1.0f, 1.0f),
    vec2(-1.0f, -1.0f)
);

const vec2 texcoords[6] = vec2[](
    vec2(0.0f, 0.0f),
    vec2(1.0f, 0.0f),
    vec2(1.0f, 1.0f),

    vec2(1.0f, 1.0f),
    vec2(0.0f, 1.0f),
    vec2(0.0f, 0.0f)
);

out VS_OUT
{
    vec2 vTexCoord;
}
vs_out;

void main()
{
    gl_Position = vec4(positions[gl_VertexID], 0.0f, 1.0f);
    vs_out.vTexCoord = texcoords[gl_VertexID];
}
