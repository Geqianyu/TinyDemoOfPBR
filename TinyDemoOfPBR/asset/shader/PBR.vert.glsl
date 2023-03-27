
#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coord;

out Vs_out
{
    vec2 texture_coord;
    vec3 position;
    vec3 normal;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vs_out.texture_coord = texture_coord;
    vs_out.position = vec3(model * vec4(position, 1.0));
    vs_out.normal = mat3(model) * normal;

    gl_Position =  projection * view * vec4(vs_out.position, 1.0);
}