#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

layout (location = 0) uniform mat4 uPVM;
layout (location = 1) uniform mat4 uModelView;

out vec3 vEyeSpace;
out vec2 vTexCoord;

void main(void)
{
    gl_Position = uPVM * vec4(aPos, 1.0f);
    vEyeSpace = vec3(uModelView * vec4(aPos, 1.0f));
    vTexCoord = aTexCoord;
}
