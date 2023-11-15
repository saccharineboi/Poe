#version 460 core

in vec3 vTexCoord;

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
    color = texture(uSkybox, vTexCoord); 
    color.rgb = pow(color.rgb, vec3(uGamma));
}
