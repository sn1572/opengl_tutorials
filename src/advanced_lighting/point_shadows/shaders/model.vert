#version 400 core

layout (location=0) in vec3 in_position;
layout (location=1) in vec3 in_normal;
layout (location=2) in vec2 in_texture_coordinates;
layout (location=3) in vec3 in_tangent;
layout (location=4) in vec3 in_bitangent;

out vec3 fragment_position;
out vec3 normal;
out vec2 texture_coordinates; 
out vec3 view_direction;
out vec3 light_direction;
out mat3 tbn_matrix;
out vec4 shadow_position;

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

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model_matrix;
uniform mat4 normal_matrix;
uniform vec3 camera_position;
uniform Light point_light;


void main(){
    fragment_position = vec3(model_matrix * vec4(in_position, 1.0));
	texture_coordinates = in_texture_coordinates;
	gl_Position = projection * view * model_matrix * vec4(in_position, 1.0);
    vec3 tangent = normalize(vec3(normal_matrix * vec4(in_tangent, 0.0)));
    vec3 bitangent = normalize(vec3(normal_matrix * vec4(in_bitangent, 0.0)));
    normal = normalize(vec3(normal_matrix * vec4(in_normal, 0.0)));
    tbn_matrix = mat3(tangent, bitangent, normal);
    mat3 tbn_transpose = transpose(tbn_matrix);
    light_direction = tbn_transpose * normalize(point_light.position \
                                                - fragment_position);
    view_direction = tbn_transpose * normalize(camera_position \
                                               - fragment_position);
}
