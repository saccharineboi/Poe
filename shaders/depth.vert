#version 460 core

layout (location = 0) in vec3 aPos;

layout (location = 0) uniform mat4 uLightMatrix;
layout (location = 1) uniform mat4 uModel;

void main()
{
    gl_Position = uLightMatrix * uModel * vec4(aPos, 1.0f);
}
