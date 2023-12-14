struct DirLight_t
{
    vec3 color;
    vec3 direction; // in view space
    float intensity;

    mat4 lightSpace0;
    mat4 lightSpace1;
    mat4 lightSpace2;
    mat4 lightSpace3;
    mat4 lightSpace4;
};
