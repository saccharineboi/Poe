#version 450 core

// assume CCW
const vec2 positions[6] = vec2[](
    vec2(-1.0f, -1.0f),
    vec2(1.0f, -1.0f),
    vec2(1.0f, 1.0f),

    vec2(1.0f, 1.0f),
    vec2(-1.0f, 1.0f),
    vec2(-1.0f, -1.0f)
);

void main()
{
    gl_Position = vec4(positions[gl_VertexID], 0.0f, 1.0f);
}
