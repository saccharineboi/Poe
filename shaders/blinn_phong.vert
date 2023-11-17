#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoord;

layout (std140, binding = 1) TransformBlock
{
    mat4 uProjection;
    mat4 uView;
    mat4 uProjView;
    vec3 uCamDir;
};

layout (location = 0) uniform mat4 uModel;
layout (location = 1) uniform mat3 uNorm;
layout (location = 2) uniform vec2 uTexOffset;
layout (location = 3) uniform vec2 uTexMultiplier;

out VS_OUT
{
    vec3 vFragPos;
    vec3 vNorm;
    vec2 vTexCoord;
}
vs_out;

void main()
{
    gl_Position = uProjView * uModel * vec4(aPos, 1.0f);
    vs_out.vFragPos = vec3(uView * uModel * vec4(aPos, 1.0f));
    vs_out.vNorm = uNorm * aNorm;
    vs_out.vTexCoord = aTexCoord * uTexMultiplier + uTexOffset; 
}
