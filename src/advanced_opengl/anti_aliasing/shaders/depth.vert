#version 450 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_texture_coordinates;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;

uniform mat4 light_space_matrix;
uniform mat4 model_matrix;

void main()
{
    gl_Position = light_space_matrix * model_matrix * vec4(in_position, 1.0);
}
