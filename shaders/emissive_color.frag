#version 460 core

in vec3 vEyeSpace;

layout (location = 0) uniform vec4 uColor;

layout (std140, binding = 0) uniform FogBlock
{
    vec4 uFogColor;
    float uFogDistance;
    float uFogExp;
};

out vec4 color;
void main(void)
{
    color = mix(uColor, uFogColor, clamp(pow(length(vEyeSpace) / uFogDistance, uFogExp), 0.0f, 1.0f));
}
