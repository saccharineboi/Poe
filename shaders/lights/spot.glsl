struct SpotLight_t
{
    vec3 color;
    vec3 direction;
    vec3 position;
    float innerCutoff;
    float outerCutoff;
    float constant;
    float linear;
    float quadratic;
    float intensity;

    mat4 lightSpace;
};
