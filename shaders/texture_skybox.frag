#version 450 core

in vec3 vTexCoord;

uniform samplerCube uSkybox;

out vec4 color;
void main()
{
    color = texture(uSkybox, vTexCoord); 
}
