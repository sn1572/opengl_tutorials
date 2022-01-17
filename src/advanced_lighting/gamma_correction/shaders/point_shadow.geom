#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

out vec4 frag_pos;

struct Light {
    vec3 position;
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

uniform Light point_light;

void main()
{
    for (int face = 0; face < 6; face++){
        gl_Layer = face;
        for (int i = 0; i < 3; i++){
            frag_pos = gl_in[i].gl_Position;
            gl_Position = point_light.cube_mats[face] * frag_pos;
            EmitVertex();
        }
        EndPrimitive();
    }
}
