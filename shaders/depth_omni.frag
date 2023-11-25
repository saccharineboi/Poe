#version 460 core

in GEOM_OUT
{
    vec4 vFragPos;
}
geom_in;

layout (location = 25) uniform float uFarPlane;
layout (location = 26) uniform vec3 uLightPos; // world space

void main()
{
    gl_FragDepth = length(geom_in.vFragPos.xyz - uLightPos) / uFarPlane;
}
