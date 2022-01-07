#version 450 core


out vec4 frag_color;


struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};


uniform Material material;
const float near = 0.1;
const float far = 100.0;


float linearized_depth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}


void main(){
    float depth = gl_FragCoord.z;
    depth = linearized_depth(depth) / far;
    frag_color = vec4(vec3(depth), 1.0);
}
