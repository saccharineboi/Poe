vec4 FixGamma(float gamma, vec4 inColor)
{
    return vec4(pow(inColor.rgb, vec3(gamma)), inColor.a);
}

////////////////////////////////////////
vec3 GammaCorrect(vec3 col, float gamma)
{
    return pow(col, vec3(1.0f / gamma));
}

#define GAMMA_INCLUDED
