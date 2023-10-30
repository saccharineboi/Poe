#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 8) in mat4 aModel;

layout (std140, binding = 1) uniform TransformBlock
{
    mat4 uProjection;
    mat4 uView;
    mat4 uProjView;
    vec3 uCamDir;
};

out vec3 vEyeSpace;
out vec2 vTexCoord;

void main(void)
{
    gl_Position = uProjView * aModel * vec4(aPos, 1.0f);
    vEyeSpace = vec3(uView * aModel * vec4(aPos, 1.0f));
    vTexCoord = aTexCoord;
}
