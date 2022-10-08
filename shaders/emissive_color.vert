#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 8) in mat4 uModel;

layout (std140, binding = 1) uniform TransformBlock
{
    mat4 uProjection;
    mat4 uView;
};

out vec3 vEyeSpace;

void main(void)
{
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0f);
    vEyeSpace = vec3(uView * uModel * vec4(aPos, 1.0f));
}
