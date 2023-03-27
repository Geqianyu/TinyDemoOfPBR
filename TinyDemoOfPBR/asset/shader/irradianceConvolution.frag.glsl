
#version 460 core

out vec4 fragment_color;

in vec3 world_position;

uniform samplerCube environment_map;

const float PI = 3.14159265359;

void main()
{
    vec3 normal = normalize(world_position);
    vec3 irradiance = vec3(0.0);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sample_delta = 0.025;
    int samples_count = 0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sample_delta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sample_delta)
        {
            vec3 tangent_sample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            vec3 sample_vector = tangent_sample.x * right + tangent_sample.y * up + tangent_sample.z * normal; 
            irradiance += texture(environment_map, sample_vector).rgb * cos(theta) * sin(theta);
            ++samples_count;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(samples_count));
    fragment_color = vec4(irradiance, 1.0);
}
