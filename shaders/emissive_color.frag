#version 450 core

in vec3 vEyeSpace;

uniform vec4 uColor;
uniform vec4 uFogColor;
uniform float uFogDistance;
uniform float uFogExp;

out vec4 color;
void main(void)
{
    color = mix(uColor, uFogColor, clamp(pow(length(vEyeSpace) / uFogDistance, uFogExp), 0.0f, 1.0f));
}
