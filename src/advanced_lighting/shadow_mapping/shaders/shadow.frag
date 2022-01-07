#version 400 core


in vec3 fragment_position;
in vec3 normal;
in vec2 texture_coordinates; 
in vec3 view_direction;
in vec3 light_direction;
in mat3 tbn_matrix;
in vec4 shadow_position;

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
};

uniform Material material;
uniform vec3 camera_position;
uniform Light point_light;
const float pi  = 3.14159265;
const float ksh = 16.0;


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


float shadow_calculation(Light light, vec4 shadow_position)
{
    /* This division performed automagically by the vert shader normally */
    vec3 projected_coordinates = shadow_position.xyz / shadow_position.w;
    /* light-space position is in [-1,1], gotta move to [0,1] */
    projected_coordinates = projected_coordinates * 0.5 + vec3(0.5);
    float closest_depth = texture(light.depth_texture,
                                  projected_coordinates.xy).r;
    float bias_max = 0.01, bias_min = 0.001;
    float bias = max(bias_max * (1.0 - dot(normal, light_direction)), bias_min);
    float current_depth = projected_coordinates.z;
    float shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;
    return shadow;
}


vec3 calc_point_light(Light light, vec3 fragment_position)
{
    float shininess_correction;
    vec3 material_texture = texture(material.texture_diffuse1,
                                    texture_coordinates).rgb;
    /* Tutorial has -light_direction here. 
     * Drawing seems to indicate it should just be light_direction.
     */
    vec3 normal_texture = texture(material.texture_normal1,
                                  texture_coordinates).rgb;
    normal_texture = normalize(2.0 * normal_texture - vec3(1.0));

    vec3 ambient = light.ambient * material_texture;
    float diff = max(dot(normal_texture, light_direction), 0.0);
    vec3 diffuse = light.diffuse * diff * material_texture;
    vec3 avg_direction = normalize(light_direction + view_direction);
    /* Blinn-Phong mode */
    shininess_correction = (8.0 + ksh) / (8.0 * pi);
    /* Phong */
    //shininess_correction = (2.0 + ksh) / (2.0 * pi);
    float spec = pow(max(dot(normal_texture, avg_direction), 0.0),
                     material.shininess);
    spec *= shininess_correction;
    vec3 specular = light.specular * spec * \
                    texture(material.texture_specular1,
                            texture_coordinates).rgb;
    float distance = length(light.position - fragment_position);
    float attenuation = 1.0 / (light.constant + light.linear * distance + \
                               light.quadratic * (distance * distance));
    float shadow = shadow_calculation(light, shadow_position);
    ambient *= attenuation;
    diffuse *= (1.0 - shadow) * attenuation;
    specular *= (1.0 - shadow) * attenuation;
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
    vec3 normalized_normal = normalize(normal);
    vec3 total;

    total = calc_point_light(point_light, fragment_position);
	frag_color = texture(material.texture_diffuse1, texture_coordinates) * \
                 vec4(total, 1.0);
}
