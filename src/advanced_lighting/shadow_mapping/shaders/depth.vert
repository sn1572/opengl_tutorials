#version 450 core

layout (location = 0) in vec3 in_position;

uniform mat4 light_space_matrix;
uniform mat4 model;

void main()
{
    gl_Position = light_space_matrix * model * vec4(in_position, 1.0);
}
