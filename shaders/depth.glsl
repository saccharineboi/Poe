#ifdef POE_VERTEX_SHADER

////////////////////////////////////////
//////////// VERTEX SHADER /////////////
////////////////////////////////////////

layout (location = POE_APOS_LOC) in vec3 aPos;

layout (location = POE_ULIGHT_MATRIX_LOC) uniform mat4 uLightMatrix;

#if POE_INSTANCED == 0
    layout (location = POE_UMODEL_LOC) uniform mat4 uModel;
#endif

#if POE_INSTANCED == 1
    layout (location = POE_AMODEL_LOC) in mat4 aModel;
#endif

#if POE_OMNI == 1
    out VS_OUT
    {
        vec3 vFragPos;
    }
    vs_out;
#endif

void main()
{
#if POE_INSTANCED == 0
    gl_Position = uLightMatrix * uModel * vec4(aPos, 1.0f);
#else
    gl_Position = uLightMatrix * aModel * vec4(aPos, 1.0f);
#endif

#if POE_OMNI == 1 && POE_INSTANCED == 1
    vs_out.vFragPos = vec3(aModel * vec4(aPos, 1.0f));
#elif POE_OMNI == 1 && POE_INSTANCED == 0
    vs_out.vFragPos = vec3(uModel * vec4(aPos, 1.0f));
#endif
}

#elif defined(POE_FRAGMENT_SHADER)

////////////////////////////////////////
//////////// FRAGMENT SHADER ///////////
////////////////////////////////////////

#if POE_OMNI == 1
    in VS_OUT
    {
        vec3 vFragPos; // world space
    }
    fs_in;

    layout (location = POE_UFAR_PLANE_LOC) uniform float uFarPlane;
    layout (location = POE_ULIGHT_POS_LOC) uniform vec3 uLightPos; // world space
#endif

void main()
{
#if POE_OMNI == 1
    gl_FragDepth = length(fs_in.vFragPos - uLightPos) / uFarPlane;
#endif
}

#endif
