struct PointLight_t
{
    vec3 color;
    vec3 worldPosition;
    vec3 viewPosition;
    float constant;
    float linear;
    float quadratic;
    float intensity;
    float nearPlane;
    float farPlane;
};
