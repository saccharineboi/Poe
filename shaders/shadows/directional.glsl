////////////////////////////////////////////////////////////////////////////////////////////////////
layout (location = POE_UDIR_LIGHT_DEPTH_MAP_LOC)  uniform sampler2DArrayShadow uDirLightDepthMap;

////////////////////////////////////////////////////////////////////////////////////////////////////
int ChooseCascade(float fragDepth, float cascadeRanges[NUM_CASCADES])
{
    for (int i = 0; i < NUM_CASCADES; ++i)
    {
        if (fragDepth < cascadeRanges[i])
        {
            return i;
        }
    }
    return NUM_CASCADES;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
float ComputeShadowForDirLights(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int layer, float farPlane, float cascadeRanges[NUM_CASCADES])
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5f + 0.5f;
    if (projCoords.z > 1.0f) {
        return 0.0f;
    }

    float biasModifier = 0.5f;
    float bias = max(SHADOW_BIAS_MAX * (1.0f - dot(normal, lightDir)), SHADOW_BIAS_MIN);

    if (layer == NUM_CASCADES)
    {
        bias *= 1.0f / (farPlane * biasModifier);
    }
    else
    {
        bias *= 1.0f / (cascadeRanges[layer] * biasModifier);
    }

    vec2 texelSize = 1.0f / vec2(textureSize(uDirLightDepthMap, 0));
    float shadow = 0.0f;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(uDirLightDepthMap, vec4(projCoords.xy + vec2(x, y) * texelSize, layer, projCoords.z - bias));
            shadow += pcfDepth;
        }
    }
    return shadow / 9.0f;
}
