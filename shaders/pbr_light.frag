#version 460 core

#define PI 3.14159265359f

#define NUM_DIR_LIGHTS 2
#define NUM_POINT_LIGHTS 4
#define NUM_SPOT_LIGHTS 4

in VS_OUT
{
    vec2 vTexCoord;
    vec3 vViewPos;
    vec3 vNormal;
} fs_in;

layout (std140, binding = 1) uniform TransformBlock
{
    mat4 uProjection;
    mat4 uView;
    mat4 uProjView;
    vec3 uCamDir;
};

layout (std140, binding = 2) uniform PBR_MaterialBlock
{
    vec3 uAlbedo;
    float uMetallic;
    float uRoughness;
    float uAO;
};

struct DirLight_t
{
    vec3 color;
    vec3 direction;
    float intensity;
};

layout (std140, binding = 3) uniform DirLightBlock
{
    DirLight_t uDirLights[NUM_DIR_LIGHTS];
};

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float den = (NdotH2 * (a2 - 1.0f) + 1.0f);
    den = PI * den * den;

    return num / den;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    return NdotV / (NdotV * (1.0f - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);

    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 CookTorranceBRDF(float NDF, float G, vec3 F, vec3 N, vec3 V, vec3 L)
{
    vec3 num = NDF * G * F;
    float den = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
    return num / den;
}

vec3 Radiance(vec3 N, vec3 L, int dirLightInd)
{
    float NdotL = max(dot(N, L), 0.0f);
    return uDirLights[dirLightInd].color * uDirLights[dirLightInd].intensity * NdotL;
}

out vec4 color;
void main(void)
{
    vec3 N = normalize(fs_in.vNormal);
    vec3 V = normalize(-fs_in.vViewPos);

    vec3 Lo = vec3(0.0f);

    for (int i = 0; i < NUM_DIR_LIGHTS; ++i) {
        vec3 L = normalize(vec3(uView * vec4(-normalize(uDirLights[i].direction), 0.0f)));
        vec3 H = normalize(V + L);
        vec3 radiance = Radiance(N, L, i);

        vec3 F0 = vec3(0.04f);
        F0 = mix(F0, uAlbedo, uMetallic);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);

        float NDF = DistributionGGX(N, H, uRoughness);
        float G = GeometrySmith(N, V, L, uRoughness);

        vec3 specular = CookTorranceBRDF(NDF, G, F, N, V, L);

        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;

        kD *= 1.0f - uMetallic;

        vec3 diffuse = kD * (uAlbedo / PI);

        Lo += radiance * (diffuse + specular);
    }
    vec3 ambient = vec3(0.03f) * uAlbedo * uAO;

    color = vec4(Lo + ambient, 1.0f);
}
