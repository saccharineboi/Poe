#version 450 core

in vec3 vTexCoord;

layout (location = 1) uniform samplerCube uSkybox;

out vec4 color;
void main()
{
    color = texture(uSkybox, vTexCoord); 
}
