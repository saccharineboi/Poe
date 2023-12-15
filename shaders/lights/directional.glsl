struct DirLight_t
{
    vec3 color;
    vec3 direction; // in view space
    float intensity;
    float farPlane;
    float cascadeRanges[NUM_CASCADES];
    mat4 lightSpace[NUM_CASCADES + 1];
};
