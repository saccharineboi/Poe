#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 8) in mat4 aModel;

layout (location = 0) uniform mat4 uLightMatrix;

void main()
{
    gl_Position = uLightMatrix * aModel * vec4(aPos, 1.0f);
}
