#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 uPVM;
uniform mat4 uModelView;

out vec3 vEyeSpace;

void main(void)
{
    gl_Position = uPVM * vec4(aPos, 1.0f);
    vEyeSpace = vec3(uModelView * vec4(aPos, 1.0f));
}
