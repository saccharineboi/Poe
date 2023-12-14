#ifdef POE_VERTEX_SHADER

////////////////////////////////////////
//////////// VERTEX SHADER /////////////
////////////////////////////////////////

// assume CCW
const vec2 positions[6] = vec2[](
    vec2(-1.0f, -1.0f),
    vec2(1.0f, -1.0f),
    vec2(1.0f, 1.0f),

    vec2(1.0f, 1.0f),
    vec2(-1.0f, 1.0f),
    vec2(-1.0f, -1.0f)
);

const vec2 texcoords[6] = vec2[](
    vec2(0.0f, 0.0f),
    vec2(1.0f, 0.0f),
    vec2(1.0f, 1.0f),

    vec2(1.0f, 1.0f),
    vec2(0.0f, 1.0f),
    vec2(0.0f, 0.0f)
);

out VS_OUT
{
    vec2 vTexCoord;
}
vs_out;

void main()
{
    gl_Position = vec4(positions[gl_VertexID], 0.0f, 1.0f);
    vs_out.vTexCoord = texcoords[gl_VertexID];
}

#elif defined(POE_FRAGMENT_SHADER)

////////////////////////////////////////
//////////// FRAGMENT SHADER ///////////
////////////////////////////////////////

in VS_OUT
{
    vec2 vTexCoord;
}
fs_in;

layout (location = POE_USCREEN_TEXTURE_LOC) uniform sampler2D uScreenTexture;
layout (location = POE_UTEXEL_STRETCH_LOC) uniform vec2 uTexelStretch;

layout (std140, binding = POE_POST_PROCESS_BLOCK_LOC) uniform PostProcessBlock
{
    float uGrayscaleWeight;
    float uKernelWeight;
    float uGamma;
    float uExposure;
    mat3 uKernel;
};

////////////////////////////////////////
vec3 ApplyGrayscale(vec3 col)
{
    float avg = 0.2126f * col.r + 0.7152f * col.g + 0.0722f * col.b;
    return vec3(avg);
}

////////////////////////////////////////
vec3 ApplyKernel()
{
    const float OFFSET = 1.0f / 300.0f;
    vec3 res = vec3(0.0f);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            res += texture(uScreenTexture, (fs_in.vTexCoord * uTexelStretch) + vec2(OFFSET * (i - 1), OFFSET * (1 - j))).rgb * uKernel[i][j];
    return res;
}

////////////////////////////////////////
vec3 ApplyExposure(vec3 col)
{
    return vec3(1.0f) - exp(-col * uExposure);
}

out vec4 color;
void main()
{
    vec3 texCol = texelFetch(uScreenTexture, ivec2(gl_FragCoord.xy * uTexelStretch), 0).rgb;
    color.rgb = mix(texCol, ApplyKernel(), uKernelWeight);
    color.rgb = mix(color.rgb, ApplyGrayscale(color.rgb), uGrayscaleWeight);
    color.rgb = ApplyExposure(color.rgb);

#ifdef GAMMA_INCLUDED
    color.rgb = GammaCorrect(color.rgb, uGamma);
#endif
    color.a = 1.0f;
}

#endif
