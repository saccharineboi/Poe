#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 uProjection;
uniform mat4 uModelView;

void main(void)
{
    gl_Position = uProjection * uModelView * vec4(aPos, 1.0f);
}
