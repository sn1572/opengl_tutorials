#version 450 core

in vec2 texture_coordinates;

out vec4 frag_color;

uniform sampler2D texture_to_render;

void main()
{
    frag_color = texture(texture_to_render, texture_coordinates);
}
