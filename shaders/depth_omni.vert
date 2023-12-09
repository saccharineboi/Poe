#version 460 core

layout (location = 0) in vec3 aPos;

layout (location = 0) uniform mat4 uModel;
layout (location = 1) uniform mat4 uLightMatrix;

out VS_OUT
{
    vec3 vFragPos;
}
vs_out;

void main()
{
    gl_Position = uLightMatrix * uModel * vec4(aPos, 1.0f);
    vs_out.vFragPos = vec3(uModel * vec4(aPos, 1.0f));
}
