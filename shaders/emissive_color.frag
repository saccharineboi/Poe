#version 450 core

in vec3 vEyeSpace;

layout (location = 2) uniform vec4 uColor;
layout (location = 3) uniform vec4 uFogColor;
layout (location = 4) uniform float uFogDistance;
layout (location = 5) uniform float uFogExp;

out vec4 color;
void main(void)
{
    color = mix(uColor, uFogColor, clamp(pow(length(vEyeSpace) / uFogDistance, uFogExp), 0.0f, 1.0f));
}
