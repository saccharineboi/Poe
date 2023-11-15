#version 460 core

in vec3 vEyeSpace;

layout (location = 0) uniform vec4 uColor;

layout (std140, binding = 0) uniform FogBlock
{
    vec4 uFogColor;
    float uFogDistance;
    float uFogExp;
};

vec3 ApplyFog(vec3 inColor)
{
    return mix(inColor, uFogColor.rgb, clamp(pow(length(vEyeSpace) / uFogDistance, uFogExp), 0.0f, 1.0f));
}

out vec4 color;
void main(void)
{
    color.rgb = ApplyFog(uColor.rgb);
    color.a = uColor.a;
}
