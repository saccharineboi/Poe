#version 460 core

#define NUM_DIR_LIGHTS 2
#define NUM_POINT_LIGHTS 4
#define NUM_SPOT_LIGHTS 4

#define NUM_CASCADES 4

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNorm;

layout (std140, binding = 1) uniform TransformBlock
{
    mat4 uProjection;
    mat4 uView;
    mat4 uProjView;
    vec3 uCamDir;
};

layout (location = 0) uniform mat4 uModel;
layout (location = 1) uniform mat3 uNorm;
layout (location = 2) uniform vec2 uTexOffset;
layout (location = 3) uniform vec2 uTexMultiplier;

struct DirLight_t
{
    vec3 color;
    vec3 direction; // in view space
    float intensity;

    mat4 lightSpace0;
    mat4 lightSpace1;
    mat4 lightSpace2;
    mat4 lightSpace3;
    mat4 lightSpace4;
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
};

layout (std140, binding = 6) uniform PointLightBlock
{
    PointLight_t uPointLights[NUM_POINT_LIGHTS];
};

struct SpotLight_t
{
    vec3 color;
    vec3 direction;
    vec3 position;
    float innerCutoff;
    float outerCutoff;
    float constant;
    float linear;
    float quadratic;
    float intensity;

    mat4 lightSpace;
};

layout (std140, binding = 7) uniform SpotLightBlock
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
        vs_out.vFragPosInDirLightSpace0[i] = uDirLights[i].lightSpace0 * uModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace1[i] = uDirLights[i].lightSpace1 * uModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace2[i] = uDirLights[i].lightSpace2 * uModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace3[i] = uDirLights[i].lightSpace3 * uModel * vec4(aPos, 1.0f);
        vs_out.vFragPosInDirLightSpace4[i] = uDirLights[i].lightSpace4 * uModel * vec4(aPos, 1.0f);
    }
}

void ComputeSpotLightSpace()
{
    for (int i = 0; i < NUM_SPOT_LIGHTS; ++i)
    {
        vs_out.vFragPosInSpotLightSpace[i] = uSpotLights[i].lightSpace * uModel * vec4(aPos, 1.0f);
    }
}

void main()
{
    gl_Position = uProjView * uModel * vec4(aPos, 1.0f);
    vs_out.vFragPos = vec3(uView * uModel * vec4(aPos, 1.0f));
    vs_out.vFragPosWorld = vec3(uModel * vec4(aPos, 1.0f));
    vs_out.vNorm = uNorm * aNorm;
    vs_out.vTexCoord = aTexCoord * uTexMultiplier + uTexOffset; 

    ComputeDirLightSpace();
    ComputeSpotLightSpace();
}
