#ifdef POE_VERTEX_SHADER

////////////////////////////////////////
//////////// VERTEX SHADER /////////////
////////////////////////////////////////

#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 2
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 4
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 4
#endif

#ifndef NUM_CASCADES
#define NUM_CASCADES 4
#endif

layout (location = POE_APOS_LOC) in vec3 aPos;
layout (location = POE_ATEXCOORD_LOC) in vec2 aTexCoord;
layout (location = POE_ANORM_LOC) in vec3 aNorm;

#if POE_INSTANCED == 1
    layout (location = POE_AMODEL_LOC) in mat4 aModel;
    layout (location = POE_ANORM_MAT_LOC) in mat3 aNormMat;
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

layout (location = POE_UNORM_LOC) uniform mat3 uNorm;
layout (location = POE_UTEX_OFFSET_LOC) uniform vec2 uTexOffset;
layout (location = POE_UTEX_MULTIPLIER_LOC) uniform vec2 uTexMultiplier;

layout (std140, binding = POE_DIR_LIGHT_BLOCK_LOC) uniform DirLightBlock
{
    DirLight_t uDirLights[NUM_DIR_LIGHTS];
};

layout (std140, binding = POE_POINT_LIGHT_BLOCK_LOC) uniform PointLightBlock
{
    PointLight_t uPointLights[NUM_POINT_LIGHTS];
};

layout (std140, binding = POE_SPOT_LIGHT_BLOCK_LOC) uniform SpotLightBlock
{
    SpotLight_t uSpotLights[NUM_SPOT_LIGHTS];
};

out VS_OUT
{
    vec3 vFragPos;
    vec3 vFragPosWorld;
    vec3 vNorm;
    vec2 vTexCoord;

    vec4 vFragPosInDirLightSpace0[NUM_DIR_LIGHTS];
    vec4 vFragPosInDirLightSpace1[NUM_DIR_LIGHTS];
    vec4 vFragPosInDirLightSpace2[NUM_DIR_LIGHTS];
    vec4 vFragPosInDirLightSpace3[NUM_DIR_LIGHTS];
    vec4 vFragPosInDirLightSpace4[NUM_DIR_LIGHTS];

    vec4 vFragPosInSpotLightSpace[NUM_SPOT_LIGHTS];
}
vs_out;

void ComputeDirLightSpace()
{
    for (int i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
#if POE_INSTANCED == 0
        vs_out.vFragPosInDirLightSpace0[i] = uDirLights[i].lightSpace0 * uModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace1[i] = uDirLights[i].lightSpace1 * uModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace2[i] = uDirLights[i].lightSpace2 * uModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace3[i] = uDirLights[i].lightSpace3 * uModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace4[i] = uDirLights[i].lightSpace4 * uModel * vec4(aPos, 1.0f);
#elif POE_INSTANCED == 1
        vs_out.vFragPosInDirLightSpace0[i] = uDirLights[i].lightSpace0 * aModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace1[i] = uDirLights[i].lightSpace1 * aModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace2[i] = uDirLights[i].lightSpace2 * aModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace3[i] = uDirLights[i].lightSpace3 * aModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace4[i] = uDirLights[i].lightSpace4 * aModel * vec4(aPos, 1.0f);
#endif
    }
}

void ComputeSpotLightSpace()
{
    for (int i = 0; i < NUM_SPOT_LIGHTS; ++i)
    {
#if POE_INSTANCED == 0
        vs_out.vFragPosInSpotLightSpace[i] = uSpotLights[i].lightSpace * uModel * vec4(aPos, 1.0f);
#elif POE_INSTANCED == 1
        vs_out.vFragPosInSpotLightSpace[i] = uSpotLights[i].lightSpace * aModel * vec4(aPos, 1.0f);
#endif
    }
}

void main()
{
#if POE_INSTANCED == 0
    gl_Position = uProjView * uModel * vec4(aPos, 1.0f);
    vs_out.vFragPos = vec3(uView * uModel * vec4(aPos, 1.0f));
    vs_out.vFragPosWorld = vec3(uModel * vec4(aPos, 1.0f));
    vs_out.vNorm = uNorm * aNorm;
#elif POE_INSTANCED == 1
    gl_Position = uProjView * aModel * vec4(aPos, 1.0f);
    vs_out.vFragPos = vec3(uView * aModel * vec4(aPos, 1.0f));
    vs_out.vFragPosWorld = vec3(aModel * vec4(aPos, 1.0f));
    vs_out.vNorm = aNormMat * aNorm;
#endif

    vs_out.vTexCoord = aTexCoord * uTexMultiplier + uTexOffset; 

    ComputeDirLightSpace();
    ComputeSpotLightSpace();
}

#elif defined(POE_FRAGMENT_SHADER)

////////////////////////////////////////
//////////// FRAGMENT SHADER ///////////
////////////////////////////////////////

#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 2
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 4
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 4
#endif

#ifndef SHADOW_BIAS_MIN
#define SHADOW_BIAS_MIN 0.01f
#endif

#ifndef SHADOW_BIAS_MAX
#define SHADOW_BIAS_MAX 0.1f
#endif

#ifndef POINT_SHADOW_BIAS
#define POINT_SHADOW_BIAS 0.005f
#endif

#ifndef NUM_CASCADES
#define NUM_CASCADES 4
#endif

#ifndef EPSILON
#define EPSILON 0.000001f
#endif

in VS_OUT
{
    vec3 vFragPos;
    vec3 vFragPosWorld;
    vec3 vNorm;
    vec2 vTexCoord;

    vec4 vFragPosInDirLightSpace0[NUM_DIR_LIGHTS];
    vec4 vFragPosInDirLightSpace1[NUM_DIR_LIGHTS];
    vec4 vFragPosInDirLightSpace2[NUM_DIR_LIGHTS];
    vec4 vFragPosInDirLightSpace3[NUM_DIR_LIGHTS];
    vec4 vFragPosInDirLightSpace4[NUM_DIR_LIGHTS];

    vec4 vFragPosInSpotLightSpace[NUM_SPOT_LIGHTS];
}
fs_in;

layout (std140, binding = POE_TRANSFORM_BLOCK_LOC) uniform TransformBlock
{
    mat4 uProjection;
    mat4 uView;
    mat4 uProjView;
    vec3 uCamDir;
};

layout (std140, binding = POE_BLINN_PHONG_MATERIAL_BLOCK_LOC) uniform BlinnPhongMaterialBlock
{
    vec3 uMaterialAmbient;
    vec3 uMaterialDiffuse;
    vec3 uMaterialSpecular;
    float uMaterialShininess;
};

layout (std140, binding = POE_DIR_LIGHT_BLOCK_LOC) uniform DirLightBlock
{
    DirLight_t uDirLights[NUM_DIR_LIGHTS];
};

layout (std140, binding = POE_POINT_LIGHT_BLOCK_LOC) uniform PointLightBlock
{
    PointLight_t uPointLights[NUM_POINT_LIGHTS];
};

layout (std140, binding = POE_SPOT_LIGHT_BLOCK_LOC) uniform SpotLightBlock
{
    SpotLight_t uSpotLights[NUM_SPOT_LIGHTS];
};

layout (std140, binding = POE_POST_PROCESS_BLOCK_LOC) uniform PostProcessBlock
{
    float uGrayscaleWeight;
    float uKernelWeight;
    float uGamma;
    float uExposure;
    mat3 uKernel;
};

layout (location = POE_UMATERIAL_AMBIENT_TEXTURE_LOC) uniform sampler2D uMaterialAmbientTexture;
layout (location = POE_UMATERIAL_DIFFUSE_TEXTURE_LOC) uniform sampler2D uMaterialDiffuseTexture;
layout (location = POE_UMATERIAL_SPECULAR_TEXTURE_LOC) uniform sampler2D uMaterialSpecularTexture;

layout (location = POE_UAMBIENT_FACTOR_LOC) uniform float uAmbientFactor;

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
        int layer = ChooseCascade(length(fs_in.vFragPos));
        if (layer == 0) {
            shadowComp = ComputeShadowForDirLights(fs_in.vFragPosInDirLightSpace0[i], normal, lightDir, layer);
        }
        else if (layer == 1) {
            shadowComp = ComputeShadowForDirLights(fs_in.vFragPosInDirLightSpace1[i], normal, lightDir, layer);
        }
        else if (layer == 2) {
            shadowComp = ComputeShadowForDirLights(fs_in.vFragPosInDirLightSpace2[i], normal, lightDir, layer);
        }
        else if (layer == 3) {
            shadowComp = ComputeShadowForDirLights(fs_in.vFragPosInDirLightSpace3[i], normal, lightDir, layer);
        }
        else if (layer == 4) {
            shadowComp = ComputeShadowForDirLights(fs_in.vFragPosInDirLightSpace4[i], normal, lightDir, layer);
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

        float shadowComp = ComputeShadowForPointLights(uPointLights[i].worldPosition, fs_in.vFragPosWorld, uPointLights[i].farPlane);

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

        float shadowComp = ComputeShadowForSpotLights(fs_in.vFragPosInSpotLightSpace[i]);
        result += shadowComp * intensity * attenuation * uSpotLights[i].intensity * (diffuse + specular);
    }
    return result;
}

out vec4 color;
void main()
{
    vec4 _ambientTexColor = texture(uMaterialAmbientTexture, fs_in.vTexCoord);
    vec4 _diffuseTexColor = texture(uMaterialDiffuseTexture, fs_in.vTexCoord);
    vec4 _specularTexColor = texture(uMaterialSpecularTexture, fs_in.vTexCoord);

    if (_diffuseTexColor.a < 0.01f) discard;

#ifdef GAMMA_INCLUDED
    vec3 ambientTexColor = FixGamma(uGamma, texture(uMaterialAmbientTexture, fs_in.vTexCoord)).rgb;
    vec3 diffuseTexColor = FixGamma(uGamma, texture(uMaterialDiffuseTexture, fs_in.vTexCoord)).rgb;
    vec3 specularTexColor = FixGamma(uGamma, texture(uMaterialSpecularTexture, fs_in.vTexCoord)).rgb;
#else
    vec3 ambientTexColor = _ambientTexColor.rgb;
    vec3 diffuseTexColor = _diffuseTexColor.rgb;
    vec3 specularTexColor = _specularTexColor.rgb;
#endif

    vec3 normal = normalize(fs_in.vNorm);
    vec3 viewDir = normalize(-fs_in.vFragPos);

    color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    color.rgb += computeDirLight(normal, fs_in.vFragPos, viewDir, diffuseTexColor, specularTexColor);
    color.rgb += computePointLight(normal, fs_in.vFragPos, viewDir, diffuseTexColor, specularTexColor);
    color.rgb += computeSpotLight(normal, fs_in.vFragPos, viewDir, diffuseTexColor, specularTexColor);

    color.rgb += uAmbientFactor * ambientTexColor * uMaterialAmbient;

#ifdef FOG_INCLUDED
    color.rgb = ApplyFog(color.rgb, fs_in.vFragPos);
#endif
}

#endif
