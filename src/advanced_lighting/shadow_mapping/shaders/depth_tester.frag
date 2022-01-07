#version 450 core


in vec3 fragment_position;
in vec3 normal;
in vec2 texture_coordinates; 

out vec4 frag_color;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float theta_min;
    float theta_taper_start;
};

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 specular;
    vec3 diffuse;
    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform vec3 camera_position;
uniform DirectionalLight directional_light;
uniform PointLight point_light;
uniform SpotLight spotlight;

const float near = 0.1;
const float far = 100.0;


vec3 calc_directional_light(DirectionalLight light, vec3 normal,
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


vec3 calc_point_light(PointLight light, vec3 normal, vec3 fragment_position,
                      vec3 view_direction)
{
    vec3 material_texture = texture(material.texture_diffuse1,
                                    texture_coordinates).rgb;
    vec3 light_direction = normalize(light.position - fragment_position);
    /* Tutorial has -light_direction here. 
     * Drawing seems to indicate it should just be light_direction.
     */
    vec3 reflect_direction = reflect(light_direction, normal);

    vec3 ambient = light.ambient * material_texture;
    float diff = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = light.diffuse * diff * material_texture;
    float spec = 0.0;
    if (dot(view_direction, normal) < 0){
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


vec3 calc_spotlight(SpotLight light, vec3 normal, vec3 fragment_position,
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
