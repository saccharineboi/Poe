#version 330 core

in vec3 vEyeSpace;
in vec2 vTexCoord;

uniform sampler2D uEmissiveTexture;
uniform vec2 uTileMultiplier;
uniform vec2 uTileOffset;

uniform vec4 uFogColor;
uniform float uFogDistance;
uniform float uFogExp;

out vec4 color;
void main(void)
{
    vec4 emissiveColor = texture(uEmissiveTexture, vTexCoord * uTileMultiplier + uTileOffset);
    if (emissiveColor.a < 0.01f) discard;

    color = mix(emissiveColor, uFogColor, clamp(pow(length(vEyeSpace) / uFogDistance, uFogExp), 0.0f, 1.0f));
}
