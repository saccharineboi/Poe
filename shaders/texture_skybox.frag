#version 460 core

in VS_OUT
{
    vec3 vTexCoord;
}
fs_in;

layout (location = 0) uniform samplerCube uSkybox;

layout (std140, binding = 4) uniform PostProcessBlock
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
    color.rgb = pow(color.rgb, vec3(uGamma));
}
