#version 460 core

in VS_OUT
{
    vec3 vColor;
}
fs_in;

out vec4 color;

void main(void)
{
    color = vec4(fs_in.vColor, 1.0f);
}
