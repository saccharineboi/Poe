#version 460 core

in vec2 vTexCoord;

layout (location = 0) uniform sampler2D uScreenTexture;

layout (std140, binding = 4) uniform PostProcessBlock
{
    float uGrayscaleWeight;
    float uKernelWeight;
    float uGamma;
    float uExposure;
    mat3 uKernel;
};

////////////////////////////////////////
vec3 makeGrayscale(vec3 col)
{
    float avg = 0.2126f * col.r + 0.7152f * col.g + 0.0722f * col.b;
    return vec3(avg);
}

////////////////////////////////////////
vec3 applyKernel()
{
    const float OFFSET = 1.0f / 300.0f;
    vec3 res = vec3(0.0f);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            res += texture(uScreenTexture, vTexCoord + vec2(OFFSET * (i - 1), OFFSET * (1 - j))).rgb * uKernel[i][j];
    return res;
}

////////////////////////////////////////
vec3 gammaCorrect(vec3 col)
{
    return pow(col, vec3(1.0f / uGamma));
}

////////////////////////////////////////
vec3 applyExposure(vec3 col)
{
    return vec3(1.0f) - exp(-col * uExposure);
}

out vec4 color;
void main()
{
    vec3 texCol = texelFetch(uScreenTexture, ivec2(gl_FragCoord.xy), 0).rgb;
    color.rgb = mix(texCol, applyKernel(), uKernelWeight);
    color.rgb = mix(color.rgb, makeGrayscale(color.rgb), uGrayscaleWeight);
    color.rgb = applyExposure(color.rgb);
    color.rgb = gammaCorrect(color.rgb);
    color.a = 1.0f;
}
