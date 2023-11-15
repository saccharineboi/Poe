#version 460 core

in vec3 vEyeSpace;
in vec2 vTexCoord;

layout (location = 0) uniform sampler2D uEmissiveTexture;
layout (location = 1) uniform vec2 uTileMultiplier;
layout (location = 2) uniform vec2 uTileOffset;

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
    vec4 emissiveColor = texture(uEmissiveTexture, vTexCoord * uTileMultiplier + uTileOffset);
    if (emissiveColor.a < 0.01f) discard;

    color.rgb = ApplyFog(emissiveColor.rgb);
    color.a = emissiveColor.a;
}
