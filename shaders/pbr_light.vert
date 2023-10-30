#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

layout (std140, binding = 1) uniform TransformBlock
{
    mat4 uProjection;
    mat4 uView;
    mat4 uProjView;
    vec3 uCamDir;
};

layout (location = 0) uniform mat4 uModel;

out VS_OUT
{
    vec2 vTexCoord;
    vec3 vViewPos;
    vec3 vNormal;
} vs_out;

void main(void)
{
    gl_Position = uProjView * uModel * vec4(aPos, 1.0f);

    vs_out.vTexCoord = aTexCoord;
    vs_out.vViewPos = vec3(uView * uModel * vec4(aPos, 1.0f));
    vs_out.vNormal = vec3(uView  * uModel * vec4(aNormal, 0.0f));
}
