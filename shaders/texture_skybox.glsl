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

in VS_OUT
{
    vec3 vTexCoord;
}
fs_in;

layout (location = POE_USKYBOX_LOC) uniform samplerCube uSkybox;

layout (std140, binding = POE_POST_PROCESS_BLOCK_LOC) uniform PostProcessBlock
{
    float uGrayscaleWeight;
    float uKernelWeight;
    float uGamma;
    float uExposure;
    mat3 uKernel;
};

out vec4 color;
void main()
{
    color = texture(uSkybox, fs_in.vTexCoord);

#ifdef GAMMA_INCLUDED
    color = FixGamma(uGamma, color);
#endif
}

#endif
