#version 450 core


struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    float shininess;
};

uniform Material material;

void main()
{
    gl_FragDepth = gl_FragCoord.z;
}
