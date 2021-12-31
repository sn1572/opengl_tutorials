#version 400 core
layout (location=0) in vec3 in_position;
layout (location=1) in vec3 in_normal;
layout (location=2) in vec2 in_texture_coordinates;
layout (location=3) in vec3 in_tangent;
layout (location=4) in vec3 in_bitangent;

out vec3 fragment_position;
out vec3 normal;
out vec2 texture_coordinates; 
out mat3 tbn_matrix;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model_matrix;
uniform mat4 normal_matrix;


void main(){
	gl_Position = projection * view * model_matrix * vec4(in_position, 1.0);
	texture_coordinates = in_texture_coordinates;
    fragment_position = vec3(model_matrix * vec4(in_position, 1.0));
    vec3 tangent = normalize(vec3(normal_matrix * vec4(in_tangent, 0.0)));
    vec3 bitangent = normalize(vec3(normal_matrix * vec4(in_bitangent, 0.0)));
    normal = normalize(vec3(normal_matrix * vec4(in_normal, 0.0)));
    tbn_matrix = mat3(tangent, bitangent, normal);
}
