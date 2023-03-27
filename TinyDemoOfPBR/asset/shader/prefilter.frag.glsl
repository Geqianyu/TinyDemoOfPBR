
#version 460 core

out vec4 fragment_color;

in vec3 world_position;

uniform samplerCube environment_map;
uniform float roughness;

const float PI = 3.14159265359;

float Trowbridge_Reitz_GGX(vec3 normal, vec3 half_vector, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float normal_dot_half_vector = max(dot(normal, half_vector), 0.0);
    float normal_dot_half_vector2 = normal_dot_half_vector * normal_dot_half_vector;
    float denom = (normal_dot_half_vector2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / denom;
}

float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10;
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

void main()
{
    vec3 normal = normalize(world_position);
    vec3 reflect_direction = normal;
    vec3 view_direction = reflect_direction;

    const uint SAMPLE_COUNT = 1024u;
    vec3 prefiltered_color = vec3(0.0);
    float total_weight = 0.0;

    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 half_vector = importance_sample_GGX(Xi, normal, roughness);
        vec3 light_direction  = normalize(2.0 * dot(view_direction, half_vector) * half_vector - view_direction);

        float normal_dot_light_direction = max(dot(normal, light_direction), 0.0);
        if(normal_dot_light_direction > 0.0)
        {
            float D   = Trowbridge_Reitz_GGX(normal, half_vector, roughness);
            float normal_dot_half_vector = max(dot(normal, half_vector), 0.0);
            float half_vector_dot_view_direction = max(dot(half_vector, view_direction), 0.0);
            float pdf = D * normal_dot_half_vector / (4.0 * half_vector_dot_view_direction) + 0.0001;
            float resolution = 512.0;
            float sa_texel  = 4.0 * PI / (6.0 * resolution * resolution);
            float sa_sample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            float mip_level = roughness == 0.0 ? 0.0 : 0.5 * log2(sa_sample / sa_texel); 
            prefiltered_color += textureLod(environment_map, light_direction, mip_level).rgb * normal_dot_light_direction;
            total_weight += normal_dot_light_direction;
        }
    }
    prefiltered_color = prefiltered_color / total_weight;
    fragment_color = vec4(prefiltered_color, 1.0);
}
