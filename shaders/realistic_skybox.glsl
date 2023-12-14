#ifdef POE_VERTEX_SHADER

////////////////////////////////////////
//////////// VERTEX SHADER /////////////
////////////////////////////////////////

// assume CCW
const vec3 positions[36] = vec3[](
    // front face
    vec3(-1.0f, -1.0f,  1.0f),
    vec3(-1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f, -1.0f,  1.0f),
    vec3(-1.0f, -1.0f,  1.0f),

    // back face
    vec3(-1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),
    vec3(-1.0f, -1.0f, -1.0f),

    // left face
    vec3(-1.0f, -1.0f, -1.0f),
    vec3(-1.0f,  1.0f, -1.0f),
    vec3(-1.0f,  1.0f,  1.0f),
    vec3(-1.0f,  1.0f,  1.0f),
    vec3(-1.0f, -1.0f,  1.0f),
    vec3(-1.0f, -1.0f, -1.0f),

    // right face
    vec3( 1.0f, -1.0f,  1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3( 1.0f, -1.0f,  1.0f),

    // top face
    vec3(-1.0f,  1.0f,  1.0f),
    vec3(-1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f, -1.0f),
    vec3( 1.0f,  1.0f,  1.0f),
    vec3(-1.0f,  1.0f,  1.0f),

    // bottom face
    vec3(-1.0f, -1.0f, -1.0f),
    vec3(-1.0f, -1.0f,  1.0f),
    vec3( 1.0f, -1.0f,  1.0f),
    vec3( 1.0f, -1.0f,  1.0f),
    vec3( 1.0f, -1.0f, -1.0f),
    vec3(-1.0f, -1.0f, -1.0f)
);

out VS_OUT
{
    vec3 vTexCoord;
}
vs_out;

layout (std140, binding = POE_TRANSFORM_BLOCK_LOC) uniform TransformBlock
{
    mat4 uProjection;
    mat4 uView;
    mat4 uProjView;
    vec3 uCamDir;
};

void main()
{
    vec4 vertexPos = uProjection * mat4(mat3(uView)) * vec4(positions[gl_VertexID], 1.0f);
    gl_Position = vertexPos.xyww;
    vs_out.vTexCoord = positions[gl_VertexID];
}

#elif defined(POE_FRAGMENT_SHADER)

////////////////////////////////////////
//////////// FRAGMENT SHADER ///////////
////////////////////////////////////////

#ifndef PI
#define PI 3.1415926f
#endif

#ifndef I_STEPS
#define I_STEPS 16
#endif

#ifndef J_STEPS
#define J_STEPS 8
#endif

in VS_OUT
{
    vec3 vTexCoord;
}
fs_in;

layout (std140, binding = POE_REALISTIC_SKYBOX_BLOCK_LOC) uniform RealisticSkyboxBlock
{
    vec3 uRayleighScatteringCoefficent;
    vec3 uRayOrigin;
    vec3 uSunPosition;
    float uSunIntensity;
    float uPlanetRadius;
    float uAtmosphereRadius;
    float uMieScatteringCoefficient;
    float uRayleighScaleHeight;
    float uMieScaleHeight;
    float uMiePreferredScatteringDirection;
};

// ray-sphere intersection where the sphere is at origin
vec2 RSI(vec3 r0, vec3 rd, float sr)
{
    float a = dot(rd, rd);
    float b = 2.0f * dot(rd, r0);
    float c = dot(r0, r0) - (sr * sr);
    float d = (b * b) - 4.0f * a * c;
    if (d < 0.0f) {
        return vec2(1e5, -1e5);
    }
    return vec2((-b - sqrt(d)) / (2.0f * a),
                (-b + sqrt(d)) / (2.0f * a));
}

vec3 Atmosphere(vec3 r,         // normalized ray direction
                vec3 r0,        // ray origin in meters
                vec3 pSun,      // position of the sun
                float iSun,     // intensity of the sun
                float rPlanet,  // radius of the planet
                float rAtmos,   // radisu of the atmosphere
                vec3 kRlh,      // Rayleigh scattering coefficient
                float kMie,     // Mie scattering coefficient
                float shRlh,    // Rayleigh scale height
                float shMie,    // Mie scale height
                float g)        // Mie preferred scattering direction
{
    pSun = normalize(pSun);
    r = normalize(r);

    // step size of primary ray
    vec2 p = RSI(r0, r, rAtmos);
    if (p.x > p.y) {
        return vec3(0.0f);
    }
    p.y = min(p.y, RSI(r0, r, rPlanet).x);
    float iStepSize = (p.y - p.x) / float(I_STEPS);

    // primary ray time
    float iTime = 0.0f;

    // accumulators for Rayleigh & Mie scattering
    vec3 totalRlh = vec3(0.0f);
    vec3 totalMie = vec3(0.0f);

    // optical depth accumulators for primary ray
    float iOdRlh = 0.0f;
    float iOdMie = 0.0f;

    // Rayleigh & Mie phases
    float mu = dot(r, pSun);
    float mumu = mu * mu;
    float gg = g * g;
    float pRlh = 3.0f / (16.0f * PI) * (1.0f + mumu);
    float pMie = 3.0f / (8.0f * PI) * ((1.0f - gg) * (mumu + 1.0f)) / (pow(1.0f + gg - 2.0f * mu * g, 1.5f) * (2.0f + gg));

    // sample primary ray
    for (int i = 0; i < I_STEPS; ++i)
    {
        // primary ray sample position
        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5f);

        // height of the sample
        float iHeight = length(iPos) - rPlanet;

        // optical depth of Rayleigh & Mie scattering
        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
        float odStepMie = exp(-iHeight / shMie) * iStepSize;

        // acculumate optical depth
        iOdRlh += odStepRlh;
        iOdMie += odStepMie;

        // step size of the secondary ray
        float jStepSize = RSI(iPos, pSun, rAtmos).y / float(J_STEPS);

        // secondary ray time
        float jTime = 0.0f;

        // optical depth accumulators for secondary ray
        float jOdRlh = 0.0f;
        float jOdMie = 0.0f;

        // sample secondary ray
        for (int j = 0; j < J_STEPS; ++j)
        {
            // secondary ray sample position
            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5f);

            // height of the sample
            float jHeight = length(jPos) - rPlanet;

            // acculumate optical depth
            jOdRlh += exp(-jHeight / shRlh) * jStepSize;
            jOdMie += exp(-jHeight / shMie) * jStepSize;

            jTime += jStepSize;
        }

        // attenuation
        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

        // accumulate scattering
        totalRlh += odStepRlh * attn;
        totalMie += odStepMie * attn;

        iTime += iStepSize;
    }
    return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);
}

out vec4 color;
void main()
{
    color.rgb = Atmosphere(normalize(fs_in.vTexCoord),
                           uRayOrigin,
                           uSunPosition,
                           uSunIntensity,
                           uPlanetRadius,
                           uAtmosphereRadius,
                           uRayleighScatteringCoefficent,
                           uMieScatteringCoefficient,
                           uRayleighScaleHeight,
                           uMieScaleHeight,
                           uMiePreferredScatteringDirection);
    color.a = 1.0f;
}

#endif
