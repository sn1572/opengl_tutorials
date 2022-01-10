#version 450 core

in vec4 frag_pos;


struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
};

uniform Material material;
uniform vec3 light_position;
uniform float far_plane;

void main()
{
    float light_distance = length(frag_pos.xyz - light_position);
    light_distance /= far_plane;
    gl_FragDepth = light_distance;
}
