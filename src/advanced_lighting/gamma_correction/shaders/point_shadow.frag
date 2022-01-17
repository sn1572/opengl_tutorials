#version 450 core

in vec4 frag_pos;


struct Light {
    vec3 position;           //not for directional
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float theta_min;         //spotlight
    float theta_taper_start; //spotlight
    float constant;          //point light
    float linear;            //point light
    float quadratic;         //point light
    mat4 shadow_matrix;
    sampler2D depth_texture;
    mat4 cube_mats[6];
};

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
};

uniform Material material;
uniform Light point_light;
uniform float far_plane;

void main()
{
    float light_distance = length(frag_pos.xyz - point_light.position);
    light_distance /= far_plane;
    gl_FragDepth = light_distance;
}
