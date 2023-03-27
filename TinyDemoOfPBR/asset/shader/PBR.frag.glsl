
#version 460 core

struct Material
{
    sampler2D albedo_map;
    sampler2D normal_map;
    sampler2D metallic_map;
    sampler2D roughness_map;
    sampler2D ao_map;
};

struct IBL
{
    samplerCube irradiance_map;
    samplerCube prefilter_map;
    sampler2D brdf_LUT_map;
};

out vec4 fragment_color;

in Vs_out
{
    vec2 texture_coord;
    vec3 position;
    vec3 normal;
} fs_in;

uniform Material material;
uniform IBL ibl;

uniform vec3 light_positions[4];
uniform vec3 light_colors[4];

uniform vec3 camera_position;

const float PI = 3.14159265359;

vec3 get_normal_from_map()
{
    vec3 tangent_normal = texture(material.normal_map, fs_in.texture_coord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.position);
    vec3 Q2  = dFdy(fs_in.position);
    vec2 st1 = dFdx(fs_in.texture_coord);
    vec2 st2 = dFdy(fs_in.texture_coord);

    vec3 N   = normalize(fs_in.normal);
    vec3 T  = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangent_normal);
}

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

vec3 Fresnel_Schlick(float cos_theta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

vec3 Fresnel_Schlick_Roughness(float cos_theta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

void main()
{
    vec3 albedo = pow(texture(material.albedo_map, fs_in.texture_coord).rgb, vec3(2.2));
    float metallic = texture(material.metallic_map, fs_in.texture_coord).r;
    float roughness = texture(material.roughness_map, fs_in.texture_coord).r;
    float ao = texture(material.ao_map, fs_in.texture_coord).r;

    vec3 normal = get_normal_from_map();
    vec3 view_direction = normalize(camera_position - fs_in.position);
    vec3 reflect_direction = reflect(-view_direction, normal); 

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i) 
    {
        vec3 light_direction = normalize(light_positions[i] - fs_in.position);
        vec3 half_vector = normalize(view_direction + light_direction);
        float distance = length(light_positions[i] - fs_in.position);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = light_colors[i] * attenuation;

        float D = Trowbridge_Reitz_GGX(normal, half_vector, roughness);   
        float G   = Geometry_Smith(normal, view_direction, light_direction, roughness);    
        vec3 F    = Fresnel_Schlick(max(dot(normal, view_direction), 0.0), F0);        

        vec3 numerator = D * G * F;
        float denominator = 4.0 * max(dot(normal, view_direction), 0.0) * max(dot(normal, light_direction), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 Ks = F;
        vec3 Kd = vec3(1.0) - Ks;
        Kd *= 1.0 - metallic;
        float normal_dot_lgiht_direction = max(dot(normal, light_direction), 0.0);

        Lo += (Kd * albedo / PI + specular) * radiance * normal_dot_lgiht_direction;
    }

    vec3 F = Fresnel_Schlick_Roughness(max(dot(normal, view_direction), 0.0), F0, roughness);
    vec3 Ks = F;
    vec3 Kd = 1.0 - Ks;
    Kd *= 1.0 - metallic;
    vec3 irradiance = texture(ibl.irradiance_map, normal).rgb;
    vec3 diffuse = irradiance * albedo;
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefiltered_color = textureLod(ibl.prefilter_map, reflect_direction,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(ibl.brdf_LUT_map, vec2(max(dot(normal, view_direction), 0.0), roughness)).rg;
    vec3 specular = prefiltered_color * (F * brdf.x + brdf.y);
    vec3 ambient = (Kd * diffuse + specular) * ao;
    vec3 color = ambient + Lo;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    fragment_color = vec4(color , 1.0);
}
