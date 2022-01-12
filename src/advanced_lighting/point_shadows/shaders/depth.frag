#version 450 core


in vec3 fragment_position;
in vec3 normal;
in vec2 texture_coordinates; 
in vec3 view_direction;
in vec3 light_direction;
in mat3 tbn_matrix;


out vec4 frag_color;


struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
};


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
    samplerCube cube_map;
};


uniform Material material;
uniform vec3 camera_position;
uniform Light point_light;
uniform float far_plane;            //Needed for cube mat calculations
const float pi  = 3.14159265;
const float ksh = 16.0;


float shadow_calculation_cube(Light light, vec3 normal)
{
    vec3 frag_to_light = fragment_position - light.position;
    float closest_depth = texture(light.cube_map, frag_to_light).r;
    closest_depth *= far_plane;
    float bias_max = 0.01, bias_min = 0.001;
    float bias = max(bias_max * (1.0 - dot(normal, light_direction)),
                     bias_min);
    float current_depth = length(frag_to_light);
    float shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;
    return shadow;
}


void main(){
    vec3 normalized_normal = normalize(normal);
    vec3 total;

	frag_color = shadow_calculation_cube(point_light, normalized_normal);
}
