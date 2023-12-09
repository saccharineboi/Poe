#version 460 core

in VS_OUT
{
    vec3 vFragPos; // world space
}
fs_in;

layout (location = 2) uniform float uFarPlane;
layout (location = 3) uniform vec3 uLightPos; // world space

void main()
{
    gl_FragDepth = length(fs_in.vFragPos - uLightPos) / uFarPlane;
}
