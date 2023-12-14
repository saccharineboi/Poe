layout (std140, binding = POE_FOG_BLOCK_LOC) uniform FogBlock
{
    vec4 uFogColor;
    float uFogDistance;
    float uFogExp;
};

vec3 ApplyFog(vec3 inColor, vec3 eyeSpace)
{
    return mix(inColor, uFogColor.rgb, clamp(pow(length(eyeSpace) / uFogDistance, uFogExp), 0.0f, 1.0f));
}

#define FOG_INCLUDED
