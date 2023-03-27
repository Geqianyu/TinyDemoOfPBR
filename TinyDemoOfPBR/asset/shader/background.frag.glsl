
#version 460 core

out vec4 fragment_color;

in vec3 world_position;

uniform samplerCube environment_map;

void main()
{
    vec3 environment_color = textureLod(environment_map, world_position, 0.0).rgb;
    environment_color = environment_color / (environment_color + vec3(1.0));
    environment_color = pow(environment_color, vec3(1.0/2.2)); 
    fragment_color = vec4(environment_color, 1.0);
}
