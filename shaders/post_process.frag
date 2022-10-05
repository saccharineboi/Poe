#version 450 core

in vec2 vTexCoord;

uniform float uGrayscaleWeight;
uniform float uKernelWeight;
uniform mat3 uKernel;
uniform sampler2D uScreenTexture;

////////////////////////////////////////
vec3 makeGrayscale(vec3 col)
{
    float avg = 0.2126f * col.r + 0.7152f * col.g + 0.0722f * col.b;
    return vec3(avg);
}

////////////////////////////////////////
vec3 applyKernel()
{
    const float texOffset = 1.0f / 300.0f;
    const vec2 offsets[9] = vec2[](
        vec2(-texOffset, texOffset), // top-left
        vec2(0.0f, texOffset), // top-center
        vec2(texOffset, texOffset), // top-right
        vec2(-texOffset, 0.0f), // center-left
        vec2(0.0f, 0.0f), // center
        vec2(texOffset, 0.0f), // center-right
        vec2(-texOffset, -texOffset), // bottom-left
        vec2(0.0f, -texOffset), // bottom-center
        vec2(texOffset, -texOffset) // bottom-right
    );

    vec3 res = vec3(0.0f);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            res += texture(uScreenTexture, vTexCoord + offsets[i * 3 + j]).rgb * uKernel[i][j];
    return res;
}

out vec4 color;
void main()
{
    vec3 texCol = texture(uScreenTexture, vTexCoord).rgb;
    color.rgb = mix(texCol, applyKernel(), uKernelWeight);
    color.rgb = mix(color.rgb, makeGrayscale(color.rgb), uGrayscaleWeight);
    color.a = 1.0f;
}
