#version 450 core

in vec2 vTexCoord;

uniform sampler2D uScreenTexture;

out vec4 color;
void main()
{
    color = texture(uScreenTexture, vTexCoord);
}
