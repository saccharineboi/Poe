#version 460 core

layout (location = 0) in vec3 aPos;

layout (std140, binding = 1) uniform TransformBlock
{
    mat4 uProjection;
    mat4 uView;
    mat4 uProjView;
    vec3 uCamDir;
};

layout (location = 1) uniform mat4 uModel;

out vec3 vEyeSpace;

void main(void)
{
    gl_Position = uProjView * uModel * vec4(aPos, 1.0f);
    vEyeSpace = vec3(uView * uModel * vec4(aPos, 1.0f));
}
