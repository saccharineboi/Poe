#ifdef POE_VERTEX_SHADER

////////////////////////////////////////
//////////// VERTEX SHADER /////////////
////////////////////////////////////////

layout (location = POE_APOS_LOC) in vec3 aPos;
layout (location = POE_ATEXCOORD_LOC) in vec2 aTexCoord;

#if POE_INSTANCED == 1
    layout (location = POE_AMODEL_LOC) in mat4 aModel;
#endif

layout (std140, binding = POE_TRANSFORM_BLOCK_LOC) uniform TransformBlock
{
    mat4 uProjection;
    mat4 uView;
    mat4 uProjView;
    vec3 uCamDir;
};

#if POE_INSTANCED == 0
    layout (location = POE_UMODEL_LOC) uniform mat4 uModel;
#endif

out VS_OUT
{
    vec3 vEyeSpace;
    vec2 vTexCoord;
}
vs_out;

void main(void)
{
#if POE_INSTANCED == 0
    gl_Position = uProjView * uModel * vec4(aPos, 1.0f);
    vs_out.vEyeSpace = vec3(uView * uModel * vec4(aPos, 1.0f));
#elif POE_INSTANCED == 1
    gl_Position = uProjView * aModel * vec4(aPos, 1.0f);
    vs_out.vEyeSpace = vec3(uView * aModel * vec4(aPos, 1.0f));
#endif
    vs_out.vTexCoord = aTexCoord;
}

#elif defined(POE_FRAGMENT_SHADER)

////////////////////////////////////////
//////////// FRAGMENT SHADER ///////////
////////////////////////////////////////

in VS_OUT
{
   vec3 vEyeSpace;
   vec2 vTexCoord;
}
fs_in;

layout (location = POE_UEMISSIVE_TEXTURE_LOC) uniform sampler2D uEmissiveTexture;
layout (location = POE_UTILE_MULTIPLIER_LOC) uniform vec2 uTileMultiplier;
layout (location = POE_UTILE_OFFSET_LOC) uniform vec2 uTileOffset;

layout (std140, binding = POE_POST_PROCESS_BLOCK_LOC) uniform PostProcessBlock
{
    float uGrayscaleWeight;
    float uKernelWeight;
    float uGamma;
    float uExposure;
    mat3 uKernel;
};

out vec4 color;
void main(void)
{
    vec4 texelColor = texture(uEmissiveTexture, fs_in.vTexCoord * uTileMultiplier + uTileOffset);
    if (texelColor.a < 0.01f) discard;

#ifdef GAMMA_INCLUDED
    texelColor = FixGamma(uGamma, texelColor);
#endif

#ifdef FOG_INCLUDED
    color.rgb = ApplyFog(emissiveColor.rgb);
#endif
}

#endif
