////////////////////////////////////////////////////////////////////////////////////////////////////
layout (location = POE_UPOINT_LIGHT_DEPTH_MAP_LOC)  uniform samplerCubeShadow uPointLightDepthMap;

////////////////////////////////////////////////////////////////////////////////////////////////////
float ComputeShadowForPointLights(vec3 lightPos, vec3 vFragPosWorld, float farPlane)
{
    vec3 fragToLight = vFragPosWorld - lightPos;
    return texture(uPointLightDepthMap, vec4(fragToLight, length(fragToLight) / farPlane - POINT_SHADOW_BIAS));
}
