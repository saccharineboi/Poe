#version 450 core

in vec3 vEyeSpace;
in vec2 vTexCoord;

layout (location = 2) uniform sampler2D uEmissiveTexture;
layout (location = 3) uniform vec2 uTileMultiplier;
layout (location = 4) uniform vec2 uTileOffset;

layout (location = 5) uniform vec4 uFogColor;
layout (location = 6) uniform float uFogDistance;
layout (location = 7) uniform float uFogExp;

out vec4 color;
void main(void)
{
    vec4 emissiveColor = texture(uEmissiveTexture, vTexCoord * uTileMultiplier + uTileOffset);
    if (emissiveColor.a < 0.01f) discard;

    color = mix(emissiveColor, uFogColor, clamp(pow(length(vEyeSpace) / uFogDistance, uFogExp), 0.0f, 1.0f));
}
