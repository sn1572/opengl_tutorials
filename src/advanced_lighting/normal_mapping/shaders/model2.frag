#version 400 core


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
};

uniform Material material;
uniform Light point_light;


vec3 calc_directional_light(Light light, vec3 normal,
                            vec3 view_direction)
{
    // These are position independent lights - eg. the sun.
    vec3 light_direction = normalize(-light.direction);
    float diff = max(dot(normal, light_direction), 0.0);
    vec3 reflect_direction = reflect(-light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0),
                     material.shininess);
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1,
                                                texture_coordinates));
    vec3 diffuse = light.diffuse * diff * \
                   vec3(texture(material.texture_diffuse1,
                        texture_coordinates));
    vec3 specular = light.specular * spec * \
                    vec3(texture(material.texture_specular1,
                                 texture_coordinates));
    return (ambient + diffuse + specular);
}


vec3 calc_point_light(Light light, vec3 fragment_position)
{
    vec3 material_texture = texture(material.texture_diffuse1,
                                    texture_coordinates).rgb;
    vec3 normal_texture = texture(material.texture_normal1,
                                  texture_coordinates).rgb;
    normal_texture = normalize(2.0 * normal_texture - vec3(1.0));
    /* R(i, n) - R is reflection, i is incident vector, n is normal vector.
     * R(i, n) by definition is i - 2(i * n)n. One can check that
     * R(i, Mn) = M R(M^Ti, n) when M is an orthogonal matrix.
     * Therefore we don't actually save ourselves a matrix multiplication
     * in the vertex shader using pure Phong.
     */
    vec3 reflect_direction = tbn_matrix * reflect(light_direction, \
                                                  normal_texture);
    vec3 ambient = light.ambient * material_texture;
    float diff = max(dot(normal_texture, light_direction), 0.0);
    vec3 diffuse = light.diffuse * diff * material_texture;
    float spec = 0.0;
    if (dot(view_direction, normal_texture) < 0){
        spec = pow(max(dot(view_direction, reflect_direction), 0.0),
                     material.shininess);
    }
    vec3 specular = light.specular * spec * \
                    texture(material.texture_specular1,
                            texture_coordinates).rgb;
    float distance = length(light.position - fragment_position);
    float attenuation = 1.0 / (light.constant + light.linear * distance + \
                               light.quadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}


vec3 calc_spotlight(Light light, vec3 normal, vec3 fragment_position,
                     vec3 view_direction)
{
    float attenuation;
    vec3 frag_to_light_direction = normalize(light.position - \
                                             fragment_position);
    float theta = dot(-frag_to_light_direction, normalize(light.direction));
    if (theta > light.theta_min){
        theta = theta-light.theta_min;
        attenuation = min(theta / (light.theta_taper_start - light.theta_min),
                          1.0);
    } else{
        attenuation = 0.0;
    }
    vec3 reflect_direction = reflect(-frag_to_light_direction, normal);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0),
                     material.shininess);
    vec3 specular = light.specular * spec * \
                    vec3(texture(material.texture_specular1,
                                 texture_coordinates));
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1,
                                                texture_coordinates));
    float diff = max(dot(normal, frag_to_light_direction), 0.0);
    vec3 diffuse = light.diffuse * diff * \
                   vec3(texture(material.texture_diffuse1,
                        texture_coordinates));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}


void main(){
    vec3 total;
    total = calc_point_light(point_light, fragment_position);
	frag_color = texture(material.texture_diffuse1, texture_coordinates) * \
                 vec4(total, 1.0);
}
