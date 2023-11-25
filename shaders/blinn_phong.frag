#version 460 core

#define NUM_DIR_LIGHTS 2
#define NUM_POINT_LIGHTS 4
#define NUM_SPOT_LIGHTS 4

#define SHADOW_BIAS_MIN 0.005f
#define SHADOW_BIAS_MAX 0.05f

in VS_OUT
{
    vec3 vFragPos;
    vec3 vFragPosWorld;
    vec4 vFragPosInDirLightSpace[NUM_DIR_LIGHTS];
    vec4 vFragPosInSpotLightSpace[NUM_SPOT_LIGHTS];
    vec3 vNorm;
    vec2 vTexCoord;
}
fs_in;

layout (std140, binding = 1) uniform TransformBlock
{
    mat4 uProjection;
    mat4 uView;
    mat4 uProjView;
    vec3 uCamDir;
};

layout (std140, binding = 5) uniform BlinnPhongMaterialBlock
{
    vec3 uMaterialAmbient;
    vec3 uMaterialDiffuse;
    vec3 uMaterialSpecular;
    float uMaterialShininess;
};

struct DirLight_t
{
    vec3 color;
    vec3 direction; // in view space
    float intensity;
    mat4 lightSpace;
    bool castShadows;
};

layout (std140, binding = 3) uniform DirLightBlock
{
    DirLight_t uDirLights[NUM_DIR_LIGHTS];
};

struct PointLight_t
{
    vec3 color;
    vec3 worldPosition;
    vec3 viewPosition;
    float constant;
    float linear;
    float quadratic;
    float intensity;
    float farPlane;
    bool castShadows;
};

layout (std140, binding = 6) uniform PointLightBlock
{
    PointLight_t uPointLights[NUM_POINT_LIGHTS];
};

struct SpotLight_t
{
    vec3 color;
    vec3 direction; // view space
    vec3 position; // view space
    float innerCutoff;
    float outerCutoff;
    float constant;
    float linear;
    float quadratic;
    float intensity;
    mat4 lightSpace;
    bool castShadows;
};

layout (std140, binding = 7) uniform SpotLightBlock
{
    SpotLight_t uSpotLights[NUM_SPOT_LIGHTS];
};

layout (std140, binding = 4) uniform PostProcessBlock
{
    float uGrayscaleWeight;
    float uKernelWeight;
    float uGamma;
    float uExposure;
    mat3 uKernel;
};

layout (location = 4) uniform sampler2D uMaterialAmbientTexture;
layout (location = 5) uniform sampler2D uMaterialDiffuseTexture;
layout (location = 6) uniform sampler2D uMaterialSpecularTexture;

layout (location = 7) uniform float uAmbientFactor;

layout (location = 8)  uniform sampler2DShadow uDirLightDepthMap;
layout (location = 9)  uniform samplerCubeShadow uPointLightDepthMap;
layout (location = 10) uniform sampler2DShadow uSpotLightDepthMap;

float ComputeShadowForDirLights(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5f + 0.5f;
    if (projCoords.z > 1.0f) {
        return 0.0f;
    }
    float bias = max(SHADOW_BIAS_MAX * (1.0f - dot(normal, lightDir)), SHADOW_BIAS_MIN);
    return texture(uDirLightDepthMap, vec3(projCoords.xy, projCoords.z - bias));
}

float ComputeShadowForPointLights(vec3 lightPos, float farPlane, vec3 normal, vec3 lightDir)
{
    vec3 fragToLight = fs_in.vFragPosWorld - lightPos;
    return texture(uPointLightDepthMap, vec4(fragToLight, length(fragToLight) / farPlane - 0.005f));
}

float ComputeShadowForSpotLights(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5f + 0.5f;
    float closestDepth = texture(uSpotLightDepthMap, projCoords);
    float currentDepth = projCoords.z;
    return currentDepth > closestDepth ? 1.0f : 0.0f;
}

vec3 computeDirLight(vec3 normal, vec3 pixelPos, vec3 viewDir, vec3 diffuseTexColor, vec3 specularTexColor)
{
    vec3 result = vec3(0.0f);
    for (int i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        vec3 lightDir = normalize(-uDirLights[i].direction);
        float diff = max(dot(normal, lightDir), 0.0f);
        vec3 diffuse = diff * uDirLights[i].color * diffuseTexColor * uMaterialDiffuse;

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0f), uMaterialShininess);
        vec3 specular = spec * uDirLights[i].color * specularTexColor * uMaterialSpecular;

        float shadowComp = 1.0f;
        if (uDirLights[i].castShadows)
        {
            shadowComp = ComputeShadowForDirLights(fs_in.vFragPosInDirLightSpace[i], normal, lightDir);
        }

        result += shadowComp * uDirLights[i].intensity * (diffuse + specular);
    }
    return result;
}

vec3 computePointLight(vec3 normal, vec3 pixelPos, vec3 viewDir, vec3 diffuseTexColor, vec3 specularTexColor)
{
    vec3 result = vec3(0.0f);
    for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
    {
        vec3 lightDir = normalize(uPointLights[i].viewPosition - pixelPos);
        float diff = max(dot(normal, lightDir), 0.0f);
        vec3 diffuse = diff * uPointLights[i].color * uMaterialDiffuse * diffuseTexColor;

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0f), uMaterialShininess);
        vec3 specular = spec * uPointLights[i].color * specularTexColor * uMaterialSpecular;

        float dist = length(uPointLights[i].viewPosition - pixelPos);
        float attenuation = 1.0f / (uPointLights[i].constant + dist * uPointLights[i].linear + dist * dist * uPointLights[i].quadratic);

        float shadowComp = 1.0f;
        if (uPointLights[i].castShadows)
        {
            shadowComp = ComputeShadowForPointLights(uPointLights[i].worldPosition, uPointLights[i].farPlane, normal, lightDir);
        }

        result += shadowComp * uPointLights[i].intensity * attenuation * (diffuse + specular);
    }
    return result;
}

vec3 computeSpotLight(vec3 normal, vec3 pixelPos, vec3 viewDir, vec3 diffuseTexColor, vec3 specularTexColor)
{
    vec3 result = vec3(0.0f);
    for (int i = 0; i < NUM_POINT_LIGHTS; ++i)
    {
        vec3 lightDir = normalize(uSpotLights[i].position - pixelPos);
        float theta = dot(lightDir, normalize(-uSpotLights[i].direction));
        float epsilon = uSpotLights[i].innerCutoff - uSpotLights[i].outerCutoff;
        float intensity = clamp((theta - uSpotLights[i].outerCutoff) / epsilon, 0.0f, 1.0f);

        float diff = max(dot(normal, lightDir), 0.0f);
        vec3 diffuse = diff * uSpotLights[i].color * uMaterialDiffuse * diffuseTexColor;

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0f), uMaterialShininess);
        vec3 specular = spec * uSpotLights[i].color * specularTexColor * uMaterialSpecular;

        float dist = length(uSpotLights[i].position - pixelPos);
        float attenuation = 1.0f / (uSpotLights[i].constant + dist * uSpotLights[i].linear + dist * dist * uSpotLights[i].quadratic);

        float shadowComp = 0.0f;
        if (uSpotLights[i].castShadows)
        {
            shadowComp = ComputeShadowForSpotLights(fs_in.vFragPosInSpotLightSpace[i]);
        }

        result += (1.0f - shadowComp) * intensity * attenuation * uSpotLights[i].intensity * (diffuse + specular);
    }
    return result;
}

vec3 FixGamma(vec3 givenColor)
{
    return pow(givenColor, vec3(uGamma));
}

layout (std140, binding = 0) uniform FogBlock
{
    vec4 uFogColor;
    float uFogDistance;
    float uFogExp;
};

vec3 ApplyFog(vec3 inColor)
{
    return mix(inColor, uFogColor.rgb, clamp(pow(length(fs_in.vFragPos) / uFogDistance, uFogExp), 0.0f, 1.0f));
}

out vec4 color;
void main()
{
    vec4 _ambientTexColor = texture(uMaterialAmbientTexture, fs_in.vTexCoord);
    vec4 _diffuseTexColor = texture(uMaterialDiffuseTexture, fs_in.vTexCoord);
    vec4 _specularTexColor = texture(uMaterialSpecularTexture, fs_in.vTexCoord);

    if (_diffuseTexColor.a < 0.01f) discard;

    vec3 ambientTexColor = FixGamma(_ambientTexColor.rgb);
    vec3 diffuseTexColor = FixGamma(_diffuseTexColor.rgb);
    vec3 specularTexColor = FixGamma(_specularTexColor.rgb);

    vec3 normal = normalize(fs_in.vNorm);
    vec3 viewDir = normalize(-fs_in.vFragPos);

    color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    color.rgb += computeDirLight(normal, fs_in.vFragPos, viewDir, diffuseTexColor, specularTexColor);
    color.rgb += computePointLight(normal, fs_in.vFragPos, viewDir, diffuseTexColor, specularTexColor);
    color.rgb += computeSpotLight(normal, fs_in.vFragPos, viewDir, diffuseTexColor, specularTexColor);

    color.rgb += uAmbientFactor * ambientTexColor * uMaterialAmbient;
    color.rgb = ApplyFog(color.rgb);
}
