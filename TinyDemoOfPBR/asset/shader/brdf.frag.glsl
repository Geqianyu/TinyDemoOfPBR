
#version 460 core

out vec2 fragment_color;
in vec2 fragment_texture_coord;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint total)
{
    return vec2(float(i)/float(total), RadicalInverse_VdC(i));
}

vec3 importance_sample_GGX(vec2 Xi, vec3 normal, float roughness)
{
    float a = roughness * roughness;
    
    float phi = 2.0 * PI * Xi.x;
    float cos_theta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    vec3 half_vector = vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
    vec3 up = abs(normal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);
    
    vec3 sample_vectorec = tangent * half_vector.x + bitangent * half_vector.y + normal * half_vector.z;
    return normalize(sample_vectorec);
}

float Geometry_Schlick_GGX(float normal_dot_view_direction, float roughness)
{
    float r = roughness + 1.0;
    float k = r * r / 8.0;
    return normal_dot_view_direction / (normal_dot_view_direction * (1.0 - k) + k);
}

float Geometry_Smith(vec3 normal, vec3 view_direction, vec3 lgiht_direction, float roughness)
{
    float normal_dot_view_direction = max(dot(normal, view_direction), 0.0);
    float normal_dot_lgiht_direction = max(dot(normal, lgiht_direction), 0.0);
    return Geometry_Schlick_GGX(normal_dot_view_direction, roughness) * Geometry_Schlick_GGX(normal_dot_lgiht_direction, roughness);
}

vec2 integrate_BRDF(float normal_dot_view_direction, float roughness)
{
    vec3 view_direction = vec3(sqrt(1.0 - normal_dot_view_direction * normal_dot_view_direction), 0.0, normal_dot_view_direction);
    float A = 0.0;
    float B = 0.0; 
    vec3 normal = vec3(0.0, 0.0, 1.0);
    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 half_vector = importance_sample_GGX(Xi, normal, roughness);
        vec3 light_direction = normalize(2.0 * dot(view_direction, half_vector) * half_vector - view_direction);
        float normal_dot_light_direction = max(light_direction.z, 0.0);
        float normal_dot_half_vector = max(half_vector.z, 0.0);
        float view_direction_dot_half_vector = max(dot(view_direction, half_vector), 0.0);

        if(normal_dot_light_direction > 0.0)
        {
            float G = Geometry_Smith(normal, view_direction, light_direction, roughness);
            float G_Vis = (G * view_direction_dot_half_vector) / (normal_dot_half_vector * normal_dot_view_direction);
            float Fc = pow(1.0 - view_direction_dot_half_vector, 5.0);
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}

void main() 
{
    vec2 integrate_BRDF_value = integrate_BRDF(fragment_texture_coord.x, fragment_texture_coord.y);
    fragment_color = integrate_BRDF_value;
}
