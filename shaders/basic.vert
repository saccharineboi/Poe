#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out VS_OUT
{
    vec3 vColor;
}
vs_out;

void main(void)
{
    gl_Position = vec4(aPos, 1.0f);
    vs_out.vColor = aColor;
}
