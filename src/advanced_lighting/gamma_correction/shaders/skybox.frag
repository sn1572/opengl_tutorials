#version 450 core


in vec3 texture_coordinates;

out vec4 frag_color;

uniform samplerCube skybox;

void main()
{
    frag_color = texture(skybox, texture_coordinates);
}
