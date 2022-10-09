#version 450 core

layout (location = 0) uniform float uGrayscaleWeight;
layout (location = 1) uniform float uKernelWeight;
layout (location = 2) uniform sampler2D uScreenTexture;
layout (location = 3) uniform float uGamma;
layout (location = 4) uniform float uExposure;

#define KERNEL_ROW_SIZE 7
#define KERNEL_SIZE (KERNEL_ROW_SIZE * KERNEL_ROW_SIZE)
layout (location = 5) uniform float uKernel[KERNEL_SIZE];

////////////////////////////////////////
vec3 makeGrayscale(vec3 col)
{
    float avg = 0.2126f * col.r + 0.7152f * col.g + 0.0722f * col.b;
    return vec3(avg);
}

////////////////////////////////////////
vec3 applyKernel()
{
    ivec2 texCoord = ivec2(gl_FragCoord.xy);
    vec3 res = vec3(0.0f);
    int cnt = 0;
    for (int i = -(KERNEL_ROW_SIZE / 2); i <= KERNEL_ROW_SIZE / 2; ++i)
        for (int j = -(KERNEL_ROW_SIZE / 2); j <= KERNEL_ROW_SIZE / 2; ++j)
            res += texelFetch(uScreenTexture, texCoord + ivec2(i, j), 0).rgb * uKernel[cnt++];
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
