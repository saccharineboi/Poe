#ifdef POE_VERTEX_SHADER

////////////////////////////////////////
//////////// VERTEX SHADER /////////////
////////////////////////////////////////

layout (location = POE_APOS_LOC) in vec3 aPos;

#if POE_INSTANCED == 1
    layout (location = POE_AMODEL_LOC) in mat4 aModel;
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

out VS_OUT
{
    vec3 vEyeSpace;
}
vs_out;

void main(void)
{
#if POE_INSTANCED == 0
    gl_Position = uProjView * uModel * vec4(aPos, 1.0f);
    vs_out.vEyeSpace = vec3(uView * uModel * vec4(aPos, 1.0f));
#elif POE_INSTANCED == 1
    gl_Position = uProjView * aModel * vec4(aPos, 1.0f);
    vs_out.vEyeSpace = vec3(uView * aModel * vec4(aPos, 1.0f));
#endif
}

#elif defined(POE_FRAGMENT_SHADER)

////////////////////////////////////////
//////////// FRAGMENT SHADER ///////////
////////////////////////////////////////

in VS_OUT
{
    vec3 vEyeSpace;
}
fs_in;

layout (location = POE_UCOLOR_LOC) uniform vec4 uColor;

out vec4 color;
void main(void)
{
    color = uColor;

#ifdef FOG_INCLUDED
    color.rgb = ApplyFog(uColor.rgb, fs_in.vEyeSpace);
#endif
}

#endif
