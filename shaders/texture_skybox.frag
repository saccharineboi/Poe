#version 460 core

in vec3 vTexCoord;

layout (location = 0) uniform samplerCube uSkybox;

out vec4 color;
void main()
{
    color = texture(uSkybox, vTexCoord); 
}
