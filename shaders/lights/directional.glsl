struct DirLight_t
{
    vec3 color;
    vec3 direction; // in view space
    float intensity;
    mat4 lightSpace[NUM_CASCADES];
};
