#version 460 core

in VS_OUT
{
   vec3 vEyeSpace;
   vec2 vTexCoord;
}
fs_in;

layout (location = 0) uniform sampler2D uEmissiveTexture;
layout (location = 1) uniform vec2 uTileMultiplier;
layout (location = 2) uniform vec2 uTileOffset;

layout (std140, binding = 0) uniform FogBlock
{
    vec4 uFogColor;
    float uFogDistance;
    float uFogExp;
};

layout (std140, binding = 4) uniform PostProcessBlock
{
    float uGrayscaleWeight;
    float uKernelWeight;
    float uGamma;
    float uExposure;
    mat3 uKernel;
};

vec3 ApplyFog(vec3 inColor)
{
    return mix(inColor, uFogColor.rgb, clamp(pow(length(fs_in.vEyeSpace) / uFogDistance, uFogExp), 0.0f, 1.0f));
}

vec4 FixGamma()
{
    vec4 emissiveColor = texture(uEmissiveTexture, fs_in.vTexCoord * uTileMultiplier + uTileOffset);
    return vec4(pow(emissiveColor.rgb, vec3(uGamma)), emissiveColor.a);
}

out vec4 color;
void main(void)
{
    vec4 emissiveColor = FixGamma();
    if (emissiveColor.a < 0.01f) discard;

    color.rgb = ApplyFog(emissiveColor.rgb);
    color.a = emissiveColor.a;
}
