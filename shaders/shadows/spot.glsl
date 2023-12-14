////////////////////////////////////////////////////////////////////////////////////////////////////
layout (location = POE_USPOT_LIGHT_DEPTH_MAP_LOC) uniform sampler2DShadow uSpotLightDepthMap;

////////////////////////////////////////////////////////////////////////////////////////////////////
float ComputeShadowForSpotLights(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5f + 0.5f;
    return texture(uSpotLightDepthMap, projCoords);
}
